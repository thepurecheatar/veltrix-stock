-- JC-Astra v2.5 | Delta MOBILE FIX | JailCrash
-- Draggable 미사용, UIGridLayout 미사용, AutomaticCanvasSize 미사용
-- 모바일 델타 100% 호환

local ok_main, err_main = pcall(function()

local Players = game:GetService("Players")
local RS = game:GetService("ReplicatedStorage")
local WS = game:GetService("Workspace")
local Http = game:GetService("HttpService")
local Run = game:GetService("RunService")
local Tween = game:GetService("TweenService")
local UIS = game:GetService("UserInputService")
local TPS = game:GetService("TeleportService")

local lp = Players.LocalPlayer
local char = lp.Character or lp.CharacterAdded:Wait()
local hrp = char:WaitForChild("HumanoidRootPart")

lp.CharacterAdded:Connect(function(c)
    char = c
    hrp = c:WaitForChild("HumanoidRootPart")
end)

------------------------------------------------------------------------
-- API
------------------------------------------------------------------------
local function R(path)
    local o = RS
    for p in path:gmatch("[^/]+") do
        if not o then return nil end
        o = o:FindFirstChild(p)
    end
    return o
end

local API = {}
local api_list = {
    {"SendTrade","API/TradeAPI/SendTradeRequest"},
    {"AcceptDecline","API/TradeAPI/AcceptOrDeclineTradeRequest"},
    {"AddItem","API/TradeAPI/AddItemToOffer"},
    {"RemoveItem","API/TradeAPI/RemoveItemFromOffer"},
    {"AcceptNeg","API/TradeAPI/AcceptNegotiation"},
    {"UnacceptNeg","API/TradeAPI/UnacceptNegotiation"},
    {"Confirm","API/TradeAPI/ConfirmTrade"},
    {"Decline","API/TradeAPI/DeclineTrade"},
    {"CanTrade","API/TradeAPI/CanTrade"},
    {"GiveItem","API/TradeAPI/GiveItem"},
    {"GetHistory","API/TradeAPI/GetTradeHistory"},
    {"ReqBackpack","API/TradeAPI/RequestBackpackAccess"},
    {"NeonFusion","API/PetAPI/DoNeonFusion"},
    {"SkipNeon","API/PetAPI/SkipNeonFusion"},
    {"Polymorph","API/PetAPI/TogglePolymorph"},
    {"Transform","API/PetAPI/SetTransformMode"},
    {"MarkHatched","API/PetAPI/MarkHatched"},
    {"ClaimReward","API/PetAPI/ClaimProgressionReward"},
    {"Equip","API/ToolAPI/Equip"},
    {"Unequip","API/ToolAPI/Unequip"},
    {"UnequipAll","API/ToolAPI/UnequipAll"},
    {"DebugStash","API/ErrorReportAPI/TradeDebugStashInfiniteConfirmation"},
    {"ExchangeItem","API/LootBoxAPI/ExchangeItemForReward"},
    {"FocusPet","API/AdoptAPI/FocusPet"},
    {"UnfocusPet","API/AdoptAPI/UnfocusPet"},
    {"RidePet","API/AdoptAPI/RidePet"},
    {"FlyPet","API/AdoptAPI/FlyPet"},
    {"ProgressAil","API/AilmentsAPI/ProgressPetMeAilment"},
    {"CollectTicket","API/PetRecyclerAPI/TicketsCollected"},
    {"IdleAddPet","API/IdleProgressionAPI/AddPet"},
    {"CreatePetObj","API/PetObjectAPI/CreatePetObject"},
    {"SuggestItem","API/TradeAPI/SuggestItem"},
    {"SetForTrade","API/PlaceableToolAPI/SetItemForTrade"},
}

local api_ok, api_miss = 0, 0
for _, v in ipairs(api_list) do
    API[v[1]] = R(v[2])
    if API[v[1]] then api_ok = api_ok + 1 else api_miss = api_miss + 1 end
end

------------------------------------------------------------------------
-- STATE
------------------------------------------------------------------------
local S = {
    target = nil, delay = 0.3, spy_on = false, spy_log = {},
    dupe_count = 0, farm_on = false, pet_esp = false, plr_esp = false,
    noclip = false, inf_jump = false, speed = 16, auto_accept = false,
    replay_seq = {}, recording = false, anti_afk = true,
}

------------------------------------------------------------------------
-- GUI 삭제 & 생성
------------------------------------------------------------------------
pcall(function() lp.PlayerGui:FindFirstChild("ASTRA"):Destroy() end)

local gui = Instance.new("ScreenGui")
gui.Name = "ASTRA"
gui.ResetOnSpawn = false
gui.Parent = lp.PlayerGui

------------------------------------------------------------------------
-- 컬러
------------------------------------------------------------------------
local C = {
    bg = Color3.fromRGB(12, 12, 20),
    card = Color3.fromRGB(28, 28, 42),
    accent = Color3.fromRGB(130, 60, 230),
    red = Color3.fromRGB(220, 50, 60),
    green = Color3.fromRGB(50, 200, 100),
    yellow = Color3.fromRGB(240, 200, 50),
    blue = Color3.fromRGB(60, 140, 240),
    text = Color3.new(1, 1, 1),
    dim = Color3.fromRGB(120, 120, 140),
    logtxt = Color3.fromRGB(140, 240, 140),
}

------------------------------------------------------------------------
-- MAIN FRAME (수동 드래그)
------------------------------------------------------------------------
local main = Instance.new("Frame")
main.Name = "M"
main.Size = UDim2.new(0, 305, 0, 460)
main.Position = UDim2.new(0, 6, 0.1, 0)
main.BackgroundColor3 = C.bg
main.BackgroundTransparency = 0.05
main.BorderSizePixel = 0
main.Active = true
main.Parent = gui

local mc = Instance.new("UICorner", main)
mc.CornerRadius = UDim.new(0, 10)

local ms = Instance.new("UIStroke", main)
ms.Color = C.accent
ms.Thickness = 1
ms.Transparency = 0.5

-- 수동 드래그 (Draggable 대체)
local dragging, dragStart, startPos
local tbar = Instance.new("Frame")
tbar.Size = UDim2.new(1, 0, 0, 30)
tbar.BackgroundColor3 = C.accent
tbar.BackgroundTransparency = 0.15
tbar.BorderSizePixel = 0
tbar.Parent = main
Instance.new("UICorner", tbar).CornerRadius = UDim.new(0, 10)

tbar.InputBegan:Connect(function(input)
    if input.UserInputType == Enum.UserInputType.Touch or input.UserInputType == Enum.UserInputType.MouseButton1 then
        dragging = true
        dragStart = input.Position
        startPos = main.Position
    end
end)

UIS.InputChanged:Connect(function(input)
    if dragging and (input.UserInputType == Enum.UserInputType.Touch or input.UserInputType == Enum.UserInputType.MouseMovement) then
        local delta = input.Position - dragStart
        main.Position = UDim2.new(startPos.X.Scale, startPos.X.Offset + delta.X, startPos.Y.Scale, startPos.Y.Offset + delta.Y)
    end
end)

UIS.InputEnded:Connect(function(input)
    if input.UserInputType == Enum.UserInputType.Touch or input.UserInputType == Enum.UserInputType.MouseButton1 then
        dragging = false
    end
end)

-- 타이틀
local ttl = Instance.new("TextLabel")
ttl.Size = UDim2.new(0.65, 0, 1, 0)
ttl.Position = UDim2.new(0, 8, 0, 0)
ttl.BackgroundTransparency = 1
ttl.Text = "JC-Astra v2.5"
ttl.TextColor3 = C.text
ttl.Font = Enum.Font.GothamBold
ttl.TextSize = 13
ttl.TextXAlignment = Enum.TextXAlignment.Left
ttl.Parent = tbar

-- 상태등
local sdot = Instance.new("Frame")
sdot.Size = UDim2.new(0, 7, 0, 7)
sdot.Position = UDim2.new(0.68, 0, 0.5, -3)
sdot.BackgroundColor3 = C.green
sdot.BorderSizePixel = 0
sdot.Parent = tbar
Instance.new("UICorner", sdot).CornerRadius = UDim.new(1, 0)

local slbl = Instance.new("TextLabel")
slbl.Size = UDim2.new(0, 35, 1, 0)
slbl.Position = UDim2.new(0.68, 10, 0, 0)
slbl.BackgroundTransparency = 1
slbl.Text = "LIVE"
slbl.TextColor3 = C.green
slbl.Font = Enum.Font.GothamBold
slbl.TextSize = 9
slbl.TextXAlignment = Enum.TextXAlignment.Left
slbl.Parent = tbar

-- 최소화
local mini = Instance.new("TextButton")
mini.Size = UDim2.new(0, 28, 0, 28)
mini.Position = UDim2.new(1, -30, 0, 1)
mini.BackgroundTransparency = 1
mini.Text = "-"
mini.TextColor3 = C.text
mini.Font = Enum.Font.GothamBold
mini.TextSize = 18
mini.ZIndex = 10
mini.Parent = tbar

------------------------------------------------------------------------
-- 토스트
------------------------------------------------------------------------
local function toast(msg, color)
    pcall(function()
        color = color or C.accent
        local t = Instance.new("Frame")
        t.Size = UDim2.new(0, 260, 0, 26)
        t.Position = UDim2.new(0.5, -130, 0, 4)
        t.BackgroundColor3 = color
        t.BackgroundTransparency = 0.15
        t.BorderSizePixel = 0
        t.ZIndex = 50
        t.Parent = gui
        Instance.new("UICorner", t).CornerRadius = UDim.new(0, 8)

        local l = Instance.new("TextLabel", t)
        l.Size = UDim2.new(1, -8, 1, 0)
        l.Position = UDim2.new(0, 4, 0, 0)
        l.BackgroundTransparency = 1
        l.Text = msg
        l.TextColor3 = C.text
        l.Font = Enum.Font.GothamBold
        l.TextSize = 11
        l.TextXAlignment = Enum.TextXAlignment.Left
        l.ZIndex = 51

        spawn(function()
            wait(2)
            pcall(function() t:Destroy() end)
        end)
    end)
end

------------------------------------------------------------------------
-- 로그 (복사 가능)
------------------------------------------------------------------------
local log_cont = Instance.new("Frame")
log_cont.Size = UDim2.new(1, -8, 0, 108)
log_cont.Position = UDim2.new(0, 4, 1, -112)
log_cont.BackgroundTransparency = 1
log_cont.Parent = main

-- 복사/클리어 버튼
local function logbtn(txt, px, color, cb)
    local b = Instance.new("TextButton")
    b.Size = UDim2.new(0, 95, 0, 18)
    b.Position = UDim2.new(0, px, 0, 0)
    b.BackgroundColor3 = color
    b.BorderSizePixel = 0
    b.Text = txt
    b.TextColor3 = C.text
    b.Font = Enum.Font.GothamBold
    b.TextSize = 9
    b.Parent = log_cont
    Instance.new("UICorner", b).CornerRadius = UDim.new(0, 4)
    b.MouseButton1Click:Connect(cb)
    return b
end

local log_history = {}
local log_n = 0

local logf = Instance.new("ScrollingFrame")
logf.Size = UDim2.new(1, 0, 0, 85)
logf.Position = UDim2.new(0, 0, 0, 21)
logf.BackgroundColor3 = Color3.fromRGB(6, 6, 10)
logf.BackgroundTransparency = 0.2
logf.BorderSizePixel = 0
logf.ScrollBarThickness = 2
logf.CanvasSize = UDim2.new(0, 0, 0, 0)
logf.Parent = log_cont

local logl = Instance.new("UIListLayout", logf)
logl.Padding = UDim.new(0, 1)
logl.SortOrder = Enum.SortOrder.LayoutOrder

local function update_canvas()
    pcall(function()
        local h = logl.AbsoluteContentSize.Y
        logf.CanvasSize = UDim2.new(0, 0, 0, h + 5)
        logf.CanvasPosition = Vector2.new(0, h + 5)
    end)
end

local function log(msg, color)
    log_n = log_n + 1
    local text = tostring(msg)
    table.insert(log_history, text)
    color = color or C.logtxt

    local l = Instance.new("TextButton")
    l.Size = UDim2.new(1, -4, 0, 12)
    l.BackgroundTransparency = 1
    l.Text = text
    l.TextColor3 = color
    l.Font = Enum.Font.Code
    l.TextSize = 9
    l.TextXAlignment = Enum.TextXAlignment.Left
    l.TextWrapped = true
    l.LayoutOrder = log_n
    l.AutoButtonColor = false
    l.Parent = logf

    l.MouseButton1Click:Connect(function()
        pcall(function() setclipboard(text) end)
        l.TextColor3 = C.yellow
        spawn(function() wait(0.4); pcall(function() l.TextColor3 = color end) end)
    end)

    update_canvas()
end

logbtn("Copy All", 0, C.green, function()
    pcall(function() setclipboard(table.concat(log_history, "\n")) end)
    toast("Copied!", C.green)
end)

logbtn("Copy 20", 98, C.blue, function()
    local s = math.max(1, #log_history - 19)
    local t = {}
    for i = s, #log_history do table.insert(t, log_history[i]) end
    pcall(function() setclipboard(table.concat(t, "\n")) end)
    toast("Last 20 copied!", C.blue)
end)

logbtn("Clear", 196, C.red, function()
    for _, c in ipairs(logf:GetChildren()) do
        if c:IsA("TextButton") then c:Destroy() end
    end
    log_history = {}; log_n = 0
    logf.CanvasSize = UDim2.new(0, 0, 0, 0)
end)

------------------------------------------------------------------------
-- 탭 시스템 (UIListLayout 없이 수동 배치)
------------------------------------------------------------------------
local tab_names = {"SPY", "DUPE", "REC", "FARM", "ESP", "CFG"}
local tab_colors = {C.blue, C.red, Color3.fromRGB(200,100,50), C.green, C.yellow, C.dim}
local tab_btns = {}
local tab_pages = {}

local tab_bar2 = Instance.new("Frame")
tab_bar2.Size = UDim2.new(1, -8, 0, 24)
tab_bar2.Position = UDim2.new(0, 4, 0, 32)
tab_bar2.BackgroundTransparency = 1
tab_bar2.Parent = main

for i, name in ipairs(tab_names) do
    local b = Instance.new("TextButton")
    b.Size = UDim2.new(0, 48, 0, 22)
    b.Position = UDim2.new(0, (i - 1) * 50, 0, 0)
    b.BackgroundColor3 = C.card
    b.BorderSizePixel = 0
    b.Text = name
    b.TextColor3 = C.dim
    b.Font = Enum.Font.GothamBold
    b.TextSize = 9
    b.Parent = tab_bar2
    Instance.new("UICorner", b).CornerRadius = UDim.new(0, 5)

    -- 페이지 (ScrollingFrame)
    local p = Instance.new("ScrollingFrame")
    p.Size = UDim2.new(1, -8, 0, 275)
    p.Position = UDim2.new(0, 4, 0, 58)
    p.BackgroundTransparency = 1
    p.BorderSizePixel = 0
    p.ScrollBarThickness = 2
    p.CanvasSize = UDim2.new(0, 0, 0, 500)
    p.Visible = false
    p.Parent = main

    tab_btns[name] = b
    tab_pages[name] = p

    b.MouseButton1Click:Connect(function()
        for n, pg in pairs(tab_pages) do
            pg.Visible = (n == name)
            tab_btns[n].BackgroundColor3 = (n == name) and tab_colors[i] or C.card
            tab_btns[n].TextColor3 = (n == name) and C.text or C.dim
        end
    end)
end

-- 버튼 팩토리 (수동 배치, 그리드 없이)
local function mkbtn(page, text, row, col, color, cb)
    color = color or C.card
    local b = Instance.new("TextButton")
    b.Size = UDim2.new(0, 143, 0, 32)
    b.Position = UDim2.new(0, col * 148, 0, row * 36)
    b.BackgroundColor3 = color
    b.BorderSizePixel = 0
    b.Text = text
    b.TextColor3 = C.text
    b.Font = Enum.Font.GothamBold
    b.TextSize = 10
    b.Parent = page
    Instance.new("UICorner", b).CornerRadius = UDim.new(0, 6)
    b.MouseButton1Click:Connect(function() pcall(cb) end)
    return b
end

------------------------------------------------------------------------
-- UTILS
------------------------------------------------------------------------
local function fire(remote, ...)
    if not remote then log("remote nil", C.red); return false end
    local ok2, err2
    if remote:IsA("RemoteEvent") then
        ok2, err2 = pcall(function() remote:FireServer(...) end)
    else
        ok2, err2 = pcall(function() remote:InvokeServer(...) end)
    end
    if not ok2 then log("ERR: " .. tostring(err2), C.red) end
    return ok2
end

local function rejoin()
    log("REJOIN...", C.yellow)
    pcall(function() TPS:TeleportToPlaceInstance(game.PlaceId, game.JobId, lp) end)
end

------------------------------------------------------------------------
-- TAB: SPY
------------------------------------------------------------------------
local sp = tab_pages["SPY"]

local old_nc
mkbtn(sp, "Net Spy ON/OFF", 0, 0, C.blue, function()
    S.spy_on = not S.spy_on
    if S.spy_on and not old_nc then
        old_nc = hookmetamethod(game, "__namecall", function(self, ...)
            local m = getnamecallmethod()
            if S.spy_on and (m == "FireServer" or m == "InvokeServer") then
                local args = {...}
                local nm = self.Name
                local low = nm:lower()
                if low:find("trade") or low:find("add") or low:find("offer")
                    or low:find("confirm") or low:find("accept") or low:find("equip")
                    or low:find("pet") or low:find("neon") or low:find("item")
                    or low:find("give") or low:find("focus") or low:find("fusion")
                    or low:find("decline") or low:find("remove") or low:find("unequip")
                    or low:find("recycle") or low:find("exchange") or low:find("hatch")
                    or low:find("backpack") or low:find("stash") then
                    local argstr = ""
                    pcall(function() argstr = Http:JSONEncode(args) end)
                    log("[SPY] " .. nm, C.blue)
                    log("  -> " .. argstr, C.dim)
                    table.insert(S.spy_log, {
                        name = nm, path = self:GetFullName(),
                        args = args, argstr = argstr, time = tick()
                    })

                    if S.recording then
                        table.insert(S.replay_seq, {
                            name = nm, path = self:GetFullName(),
                            args = args, time = tick()
                        })
                    end
                end
            end
            return old_nc(self, ...)
        end)
    end
    log("Spy: " .. (S.spy_on and "ON" or "OFF"), S.spy_on and C.green or C.red)
    toast("Spy " .. (S.spy_on and "ON" or "OFF"), S.spy_on and C.green or C.red)
end)

mkbtn(sp, "Show Captured", 0, 1, C.card, function()
    log("== " .. #S.spy_log .. " captured ==", C.yellow)
    for i, e in ipairs(S.spy_log) do
        log(i .. ") " .. e.name .. " " .. e.argstr)
    end
end)

mkbtn(sp, "Copy Captured", 1, 0, C.green, function()
    local t = {}
    for i, e in ipairs(S.spy_log) do
        table.insert(t, i .. ") " .. e.name .. " | " .. e.argstr)
    end
    pcall(function() setclipboard(table.concat(t, "\n")) end)
    toast("Copied!", C.green)
end)

mkbtn(sp, "Clear Spy", 1, 1, C.red, function()
    S.spy_log = {}; log("spy cleared")
end)

local tidx = 0
mkbtn(sp, "Next Target", 2, 0, C.accent, function()
    local o = {}
    for _, p in ipairs(Players:GetPlayers()) do
        if p ~= lp then table.insert(o, p) end
    end
    if #o == 0 then log("no players", C.red); return end
    tidx = tidx + 1
    if tidx > #o then tidx = 1 end
    S.target = o[tidx]
    log("TARGET: " .. S.target.Name, C.accent)
    toast("Target: " .. S.target.Name, C.accent)
end)

mkbtn(sp, "All Players", 2, 1, C.card, function()
    for i, p in ipairs(Players:GetPlayers()) do
        if p ~= lp then log(i .. ") " .. p.Name .. " [" .. p.UserId .. "]") end
    end
end)

mkbtn(sp, "API Status", 3, 0, C.card, function()
    log("API: " .. api_ok .. " OK / " .. api_miss .. " miss", api_ok > api_miss and C.green or C.yellow)
    for _, v in ipairs(api_list) do
        if not API[v[1]] then log("  MISS: " .. v[1], C.red) end
    end
end)

mkbtn(sp, "Full Dump", 3, 1, C.card, function()
    local cnt = 0
    for _, obj in ipairs(RS:GetDescendants()) do
        if obj:IsA("RemoteEvent") or obj:IsA("RemoteFunction") then cnt = cnt + 1 end
    end
    log("RS remotes: " .. cnt)
end)

------------------------------------------------------------------------
-- TAB: DUPE
------------------------------------------------------------------------
local dp = tab_pages["DUPE"]

mkbtn(dp, "M1: Send Trade", 0, 0, C.red, function()
    if not S.target then log("target first (SPY)", C.red); toast("No target!", C.red); return end
    log("trade -> " .. S.target.Name, C.red)
    fire(API.SendTrade, S.target)
    toast("Trade sent!", C.red)
end)

mkbtn(dp, "M1: Add Pet", 0, 1, C.card, function()
    local found
    for _, e in ipairs(S.spy_log) do
        if e.name == "AddItemToOffer" then found = e; break end
    end
    if found then
        fire(API.AddItem, unpack(found.args))
        log("pet added (captured)", C.green)
    else
        log("spy ON + 직접 펫 넣어봐", C.yellow)
    end
end)

mkbtn(dp, "M1: Accept", 1, 0, C.card, function()
    fire(API.AcceptNeg)
    log("accepted", C.green)
end)

mkbtn(dp, "M1: FIRE!", 1, 1, Color3.fromRGB(255, 30, 30), function()
    S.dupe_count = S.dupe_count + 1
    log("=== FIRE #" .. S.dupe_count .. " | d:" .. S.delay .. "s ===", C.red)
    fire(API.Confirm)
    wait(S.delay)
    rejoin()
end)

mkbtn(dp, "M2: Desync 30x", 2, 0, C.blue, function()
    log("== DESYNC 30x ==", C.blue)
    local eq, uq
    for _, e in ipairs(S.spy_log) do
        if e.name == "Equip" then eq = e.args end
        if e.name == "Unequip" or e.name == "UnequipAll" then uq = e.args end
    end
    for i = 1, 30 do
        if eq then fire(API.Equip, unpack(eq)) end
        if uq then fire(API.Unequip, unpack(uq)) else fire(API.UnequipAll) end
        wait(0.02)
    end
    log("desync done", C.green)
    toast("Desync!", C.blue)
end)

mkbtn(dp, "M2: Desync 100x", 2, 1, C.blue, function()
    log("== DESYNC 100x ==", C.red)
    local eq, uq
    for _, e in ipairs(S.spy_log) do
        if e.name == "Equip" then eq = e.args end
        if e.name == "Unequip" or e.name == "UnequipAll" then uq = e.args end
    end
    for i = 1, 100 do
        if eq then fire(API.Equip, unpack(eq)) end
        if uq then fire(API.Unequip, unpack(uq)) else fire(API.UnequipAll) end
        wait(0.01)
    end
    log("100x done", C.green)
end)

mkbtn(dp, "M3: Neon Dupe", 3, 0, C.green, function()
    log("== NEON DUPE ==", C.green)
    local nargs
    for _, e in ipairs(S.spy_log) do
        if e.name == "DoNeonFusion" then nargs = e.args; break end
    end
    if nargs then
        fire(API.NeonFusion, unpack(nargs))
        wait(S.delay)
        rejoin()
    else
        log("neon cave + spy ON 먼저", C.yellow)
    end
end)

mkbtn(dp, "M4: DebugStash", 3, 1, C.yellow, function()
    if not API.DebugStash then log("not found", C.red); return end
    fire(API.DebugStash)
    fire(API.DebugStash, true)
    fire(API.DebugStash, lp.UserId)
    fire(API.DebugStash, "enable")
    fire(API.DebugStash, lp, true)
    log("5 variants fired", C.yellow)
    toast("DebugStash!", C.yellow)
end)

mkbtn(dp, "M5: GiveItem", 4, 0, C.accent, function()
    local found
    for _, e in ipairs(S.spy_log) do
        if e.name == "GiveItem" then found = e; break end
    end
    if found then
        fire(API.GiveItem, unpack(found.args))
        wait(S.delay)
        rejoin()
    else
        log("GiveItem 미캡쳐", C.yellow)
    end
end)

mkbtn(dp, "Rejoin", 4, 1, C.red, function() rejoin() end)

------------------------------------------------------------------------
-- TAB: REC
------------------------------------------------------------------------
local rp = tab_pages["REC"]

mkbtn(rp, "Record ON/OFF", 0, 0, C.red, function()
    S.recording = not S.recording
    if S.recording then
        S.replay_seq = {}
        log("RECORDING...", C.red)
        toast("REC started!", C.red)
    else
        log("recorded " .. #S.replay_seq .. " actions", C.green)
        toast(#S.replay_seq .. " actions recorded", C.green)
    end
end)

mkbtn(rp, "Play Sequence", 0, 1, C.green, function()
    if #S.replay_seq == 0 then log("nothing", C.red); return end
    log("== REPLAY " .. #S.replay_seq .. " ==", C.yellow)
    for i, a in ipairs(S.replay_seq) do
        local remote = R(a.path:gsub("ReplicatedStorage%.", ""):gsub("%.", "/"))
        if remote then fire(remote, unpack(a.args or {})) end
        if i < #S.replay_seq then
            wait(math.min(math.max(S.replay_seq[i+1].time - a.time, 0.01), 2))
        end
    end
    log("replay done", C.green)
end)

mkbtn(rp, "Play + Rejoin", 1, 0, C.red, function()
    if #S.replay_seq == 0 then return end
    for i, a in ipairs(S.replay_seq) do
        local remote = R(a.path:gsub("ReplicatedStorage%.", ""):gsub("%.", "/"))
        if remote then fire(remote, unpack(a.args or {})) end
        if i < #S.replay_seq then
            wait(math.min(math.max(S.replay_seq[i+1].time - a.time, 0.01), 1))
        end
    end
    wait(S.delay); rejoin()
end)

mkbtn(rp, "Play 2x Speed", 1, 1, C.blue, function()
    if #S.replay_seq == 0 then return end
    for i, a in ipairs(S.replay_seq) do
        local remote = R(a.path:gsub("ReplicatedStorage%.", ""):gsub("%.", "/"))
        if remote then fire(remote, unpack(a.args or {})) end
        if i < #S.replay_seq then
            wait(math.min((S.replay_seq[i+1].time - a.time) / 2, 0.5))
        end
    end
    log("2x done", C.green)
end)

mkbtn(rp, "Show Seq", 2, 0, C.card, function()
    log("== SEQ: " .. #S.replay_seq .. " ==", C.yellow)
    for i, a in ipairs(S.replay_seq) do log(i .. ") " .. a.name) end
end)

mkbtn(rp, "Copy Seq", 2, 1, C.green, function()
    local t = {}
    for i, a in ipairs(S.replay_seq) do
        local as = ""
        pcall(function() as = Http:JSONEncode(a.args) end)
        table.insert(t, i .. ") " .. a.name .. " | " .. as)
    end
    pcall(function() setclipboard(table.concat(t, "\n")) end)
    toast("Seq copied!", C.green)
end)

mkbtn(rp, "Clear Seq", 3, 0, C.red, function()
    S.replay_seq = {}; log("cleared")
end)

mkbtn(rp, "Spam Last x20", 3, 1, C.red, function()
    if #S.spy_log == 0 then log("no data", C.red); return end
    local last = S.spy_log[#S.spy_log]
    local remote = R(last.path:gsub("ReplicatedStorage%.", ""):gsub("%.", "/"))
    if not remote then log("not found", C.red); return end
    log("spam " .. last.name .. " x20", C.red)
    for i = 1, 20 do fire(remote, unpack(last.args or {})); wait(0.02) end
    log("done", C.green)
end)

------------------------------------------------------------------------
-- TAB: FARM
------------------------------------------------------------------------
local fp = tab_pages["FARM"]
local farm_conn

mkbtn(fp, "Auto Farm", 0, 0, C.green, function()
    S.farm_on = not S.farm_on
    if S.farm_on then
        pcall(function() char:FindFirstChildOfClass("Humanoid").WalkSpeed = S.speed end)
        farm_conn = Run.Heartbeat:Connect(function()
            if not S.farm_on then return end
            pcall(function()
                for _,obj in ipairs(WS:GetDescendants()) do
                    if obj:IsA("ProximityPrompt") and obj.Enabled then
                        local p2 = obj.Parent
                        if p2 and p2:IsA("BasePart") and (hrp.Position - p2.Position).Magnitude < 60 then
                            fireproximityprompt(obj)
                        end
                    end
                end
            end)
        end)
    else
        if farm_conn then farm_conn:Disconnect(); farm_conn = nil end
    end
    log("farm: " .. (S.farm_on and "ON" or "OFF"))
    toast("Farm " .. (S.farm_on and "ON" or "OFF"), C.green)
end)

mkbtn(fp, "Auto Age Pet", 0, 1, C.card, function()
    log("auto age: 50 ticks...", C.green)
    spawn(function()
        for i = 1, 50 do
            pcall(function() fire(API.ProgressAil) end)
            wait(1)
        end
        log("age done")
    end)
end)

mkbtn(fp, "Collect Tickets", 1, 0, C.yellow, function()
    fire(API.CollectTicket); log("tickets"); toast("Tickets!", C.yellow)
end)

mkbtn(fp, "Speed 50", 1, 1, C.card, function()
    S.speed = 50
    pcall(function() char:FindFirstChildOfClass("Humanoid").WalkSpeed = 50 end)
    log("speed: 50")
end)

mkbtn(fp, "Speed 100", 2, 0, C.card, function()
    S.speed = 100
    pcall(function() char:FindFirstChildOfClass("Humanoid").WalkSpeed = 100 end)
    log("speed: 100")
end)

mkbtn(fp, "Auto Accept", 2, 1, C.accent, function()
    S.auto_accept = not S.auto_accept
    log("auto accept: " .. (S.auto_accept and "ON" or "OFF"))
    toast("AutoAccept " .. (S.auto_accept and "ON" or "OFF"), C.accent)
end)

mkbtn(fp, "Server Hop", 3, 0, C.red, function()
    log("hopping...", C.yellow)
    pcall(function() TPS:Teleport(game.PlaceId, lp) end)
end)

mkbtn(fp, "Anti-AFK", 3, 1, C.card, function()
    S.anti_afk = not S.anti_afk
    log("anti-afk: " .. (S.anti_afk and "ON" or "OFF"))
end)

-- auto accept watcher
pcall(function()
    lp.PlayerGui.DescendantAdded:Connect(function(obj)
        if not S.auto_accept then return end
        pcall(function()
            wait(0.3)
            if (obj:IsA("TextButton") or obj:IsA("ImageButton")) then
                local n = obj.Name:lower()
                if n:find("accept") or n:find("confirm") then
                    firesignal(obj.Activated)
                    log("[AUTO] " .. obj.Name, C.green)
                end
            end
        end)
    end)
end)

------------------------------------------------------------------------
-- TAB: ESP
------------------------------------------------------------------------
local ep = tab_pages["ESP"]

local nc_conn
mkbtn(ep, "Noclip", 0, 0, C.card, function()
    S.noclip = not S.noclip
    if S.noclip then
        nc_conn = Run.Stepped:Connect(function()
            pcall(function()
                for _,p2 in ipairs(char:GetDescendants()) do
                    if p2:IsA("BasePart") then p2.CanCollide = false end
                end
            end)
        end)
    else
        if nc_conn then nc_conn:Disconnect(); nc_conn = nil end
    end
    log("noclip: " .. (S.noclip and "ON" or "OFF"))
end)

mkbtn(ep, "Inf Jump", 0, 1, C.card, function()
    S.inf_jump = not S.inf_jump
    log("inf jump: " .. (S.inf_jump and "ON" or "OFF"))
end)

UIS.JumpRequest:Connect(function()
    if S.inf_jump then
        pcall(function() char:FindFirstChildOfClass("Humanoid"):ChangeState(Enum.HumanoidStateType.Jumping) end)
    end
end)

mkbtn(ep, "Pet ESP", 1, 0, C.green, function()
    S.pet_esp = not S.pet_esp
    log("pet esp: " .. (S.pet_esp and "ON" or "OFF"))
end)

mkbtn(ep, "Player ESP", 1, 1, C.red, function()
    S.plr_esp = not S.plr_esp
    log("player esp: " .. (S.plr_esp and "ON" or "OFF"))
end)

mkbtn(ep, "TP Nursery", 2, 0, C.card, function()
    pcall(function()
        hrp.CFrame = CFrame.new(-250, 30, -450)
    end)
    log("tp nursery")
end)

mkbtn(ep, "TP Pet Shop", 2, 1, C.card, function()
    pcall(function() hrp.CFrame = CFrame.new(-100, 30, 50) end)
    log("tp pet shop")
end)

mkbtn(ep, "TP Neon Cave", 3, 0, C.card, function()
    pcall(function() hrp.CFrame = CFrame.new(600, 30, 400) end)
    log("tp neon cave")
end)

mkbtn(ep, "TP School", 3, 1, C.card, function()
    pcall(function() hrp.CFrame = CFrame.new(-400, 30, 100) end)
    log("tp school")
end)

-- ESP loop
local esp_o = {}
spawn(function()
    while wait(2.5) do pcall(function()
        for _,o in ipairs(esp_o) do pcall(function() o:Destroy() end) end
        esp_o = {}

        if S.plr_esp then
            for _,p in ipairs(Players:GetPlayers()) do
                if p ~= lp and p.Character then
                    local h = p.Character:FindFirstChild("Head")
                    if h and hrp then
                        local bb = Instance.new("BillboardGui")
                        bb.Size = UDim2.new(0, 150, 0, 28)
                        bb.AlwaysOnTop = true
                        bb.StudsOffset = Vector3.new(0, 3, 0)
                        bb.Parent = h
                        local l = Instance.new("TextLabel", bb)
                        l.Size = UDim2.new(1, 0, 1, 0)
                        l.BackgroundTransparency = 1
                        l.TextColor3 = C.red
                        l.TextStrokeTransparency = 0
                        l.Font = Enum.Font.GothamBold
                        l.TextSize = 12
                        l.Text = p.Name .. " [" .. math.floor((hrp.Position - h.Position).Magnitude) .. "]"
                        table.insert(esp_o, bb)
                    end
                end
            end
        end

        if S.pet_esp then
            for _,obj in ipairs(WS:GetDescendants()) do
                if obj:IsA("Model") then
                    local nm = obj.Name:lower()
                    if nm:find("pet") or nm:find("neon") or nm:find("mega")
                        or nm:find("dragon") or nm:find("unicorn") or nm:find("frost") then
                        local r = obj:FindFirstChild("HumanoidRootPart") or obj:FindFirstChildOfClass("BasePart")
                        if r then
                            local bb = Instance.new("BillboardGui")
                            bb.Size = UDim2.new(0, 120, 0, 24)
                            bb.AlwaysOnTop = true
                            bb.StudsOffset = Vector3.new(0, 2, 0)
                            bb.Parent = r
                            local l = Instance.new("TextLabel", bb)
                            l.Size = UDim2.new(1, 0, 1, 0)
                            l.BackgroundTransparency = 1
                            l.TextColor3 = C.green
                            l.TextStrokeTransparency = 0
                            l.Font = Enum.Font.GothamBold
                            l.TextSize = 11
                            l.Text = obj.Name
                            table.insert(esp_o, bb)
                        end
                    end
                end
            end
        end
    end) end
end)

------------------------------------------------------------------------
-- TAB: CFG
------------------------------------------------------------------------
local cp = tab_pages["CFG"]

mkbtn(cp, "Delay -0.1", 0, 0, C.card, function()
    S.delay = math.max(0.02, S.delay - 0.1)
    log("delay: " .. string.format("%.2f", S.delay) .. "s")
end)

mkbtn(cp, "Delay +0.1", 0, 1, C.card, function()
    S.delay = math.min(3.0, S.delay + 0.1)
    log("delay: " .. string.format("%.2f", S.delay) .. "s")
end)

mkbtn(cp, "0.02s ULTRA", 1, 0, C.red, function()
    S.delay = 0.02
    log("ULTRA: 0.02s", C.red)
    toast("ULTRA MODE!", C.red)
end)

mkbtn(cp, "0.1s Fast", 1, 1, C.yellow, function()
    S.delay = 0.1; log("delay: 0.1s")
end)

mkbtn(cp, "0.3s Normal", 2, 0, C.green, function()
    S.delay = 0.3; log("delay: 0.3s")
end)

mkbtn(cp, "0.5s Safe", 2, 1, C.blue, function()
    S.delay = 0.5; log("delay: 0.5s")
end)

mkbtn(cp, "Export Config", 3, 0, C.green, function()
    local cfg = string.format("delay=%.2f|speed=%d|afk=%s", S.delay, S.speed, tostring(S.anti_afk))
    pcall(function() setclipboard(cfg) end)
    log("config copied: " .. cfg)
    toast("Config copied!", C.green)
end)

mkbtn(cp, "Close GUI", 3, 1, C.red, function()
    gui:Destroy()
end)

-- anti-afk loop
spawn(function()
    while wait(55) do
        if S.anti_afk then
            pcall(function()
                local vu = game:GetService("VirtualUser")
                vu:CaptureController()
                vu:ClickButton2(Vector2.new())
            end)
        end
    end
end)

------------------------------------------------------------------------
-- MINIMIZE
------------------------------------------------------------------------
local is_mini = false
mini.MouseButton1Click:Connect(function()
    is_mini = not is_mini
    for _, pg in pairs(tab_pages) do pg.Visible = false end
    tab_bar2.Visible = not is_mini
    log_cont.Visible = not is_mini

    if not is_mini then
        -- 현재 탭 다시 보이기
        for n, b in pairs(tab_btns) do
            if b.TextColor3 == C.text then
                tab_pages[n].Visible = true
                break
            end
        end
    end

    main.Size = is_mini and UDim2.new(0, 305, 0, 30) or UDim2.new(0, 305, 0, 460)
    mini.Text = is_mini and "+" or "-"
end)

------------------------------------------------------------------------
-- INIT
------------------------------------------------------------------------
tab_btns["SPY"].BackgroundColor3 = C.blue
tab_btns["SPY"].TextColor3 = C.text
tab_pages["SPY"].Visible = true

log("JC-Astra v2.5", C.accent)
log("API: " .. api_ok .. "/" .. (api_ok + api_miss), api_ok > api_miss and C.green or C.yellow)
log("SPY > DUPE > REC > FARM > ESP > CFG", C.dim)
log("dupes: " .. S.dupe_count, C.red)
toast("JC-Astra v2.5!", C.accent)

end) -- pcall 끝

-- 메인 pcall 실패시 에러 표시
if not ok_main then
    pcall(function()
        local eg = Instance.new("ScreenGui")
        eg.Name = "ASTRA_ERR"
        eg.Parent = game:GetService("Players").LocalPlayer.PlayerGui

        local ef = Instance.new("TextLabel")
        ef.Size = UDim2.new(0, 300, 0, 100)
        ef.Position = UDim2.new(0.5, -150, 0.4, 0)
        ef.BackgroundColor3 = Color3.fromRGB(40, 10, 10)
        ef.BorderSizePixel = 0
        ef.Text = "ASTRA ERROR:\n" .. tostring(err_main)
        ef.TextColor3 = Color3.fromRGB(255, 100, 100)
        ef.Font = Enum.Font.Code
        ef.TextSize = 11
        ef.TextWrapped = true
        ef.Parent = eg
    end)
end
