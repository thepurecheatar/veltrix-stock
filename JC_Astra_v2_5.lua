-- ╔══════════════════════════════════════════════════════════════╗
-- ║  JC-Astra v3.0 LEGENDARY | Ultimate AdoptMe Suite          ║
-- ║  Delta Mobile | JailCrash                                   ║
-- ║  100x Feature Upgrade | loadstring delivery                 ║
-- ╚══════════════════════════════════════════════════════════════╝

local ok_main, err_main = pcall(function()

------------------------------------------------------------------------
-- SERVICES
------------------------------------------------------------------------
local Players = game:GetService("Players")
local RS = game:GetService("ReplicatedStorage")
local WS = game:GetService("Workspace")
local Http = game:GetService("HttpService")
local Run = game:GetService("RunService")
local Tw = game:GetService("TweenService")
local UIS = game:GetService("UserInputService")
local TPS = game:GetService("TeleportService")

local lp = Players.LocalPlayer
local char = lp.Character or lp.CharacterAdded:Wait()
local hrp = char:WaitForChild("HumanoidRootPart")

lp.CharacterAdded:Connect(function(c)
    char = c; hrp = c:WaitForChild("HumanoidRootPart")
end)

------------------------------------------------------------------------
-- API RESOLVE
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
local api_defs = {
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
    {"QuickChat","API/TradeAPI/SendQuickChat"},
    {"SuggestItem","API/TradeAPI/SuggestItem"},
    {"RemoveSuggestion","API/TradeAPI/RemoveSuggestion"},
    {"ReactTrade","API/TradeAPI/ReactToTrade"},
    {"SpectateTrade","API/TradeAPI/SpectateTrade"},
    {"ReportScam","API/TradeAPI/ReportScam"},
    {"BusyIndication","API/TradeAPI/ToggleBusyIndication"},
    {"BeginQuiz","API/TradeAPI/BeginQuiz"},
    {"EndQuiz","API/TradeAPI/EndQuiz"},
    {"AnswerQuiz","API/TradeAPI/AnswerQuizQuestion"},
    {"NeonFusion","API/PetAPI/DoNeonFusion"},
    {"SkipNeon","API/PetAPI/SkipNeonFusion"},
    {"Polymorph","API/PetAPI/TogglePolymorph"},
    {"Transform","API/PetAPI/SetTransformMode"},
    {"MarkHatched","API/PetAPI/MarkHatched"},
    {"ClaimReward","API/PetAPI/ClaimProgressionReward"},
    {"PreviewReward","API/PetAPI/PreviewProgressionRewards"},
    {"PreviewLoot","API/PetAPI/PreviewProgressionLoot"},
    {"PetTrick","API/PetAPI/UpdatePetTrickLevel"},
    {"SayTrick","API/PetAPI/SayPetTrickMessage"},
    {"PetPetted","API/PetAPI/PetPetted"},
    {"BufferCmd","API/PetAPI/BufferPetCommand"},
    {"EquipLeash","API/PetAPI/EquipPetLeash"},
    {"UnequipLeash","API/PetAPI/UnequipPetLeash"},
    {"ConsumeFoodObj","API/PetAPI/ConsumeFoodObject"},
    {"PaintPet","API/PetPaintAPI/PaintPet"},
    {"SealPaint","API/PetPaintAPI/SealPetPaint"},
    {"BuyPaint","API/PetPaintAPI/BuyPetPaint"},
    {"WipeHairSpray","API/PetPaintAPI/WipeColoredHairSpray"},
    {"Equip","API/ToolAPI/Equip"},
    {"Unequip","API/ToolAPI/Unequip"},
    {"UnequipAll","API/ToolAPI/UnequipAll"},
    {"DebugStash","API/ErrorReportAPI/TradeDebugStashInfiniteConfirmation"},
    {"SendError","API/ErrorReportAPI/SendUniqueError"},
    {"FocusPet","API/AdoptAPI/FocusPet"},
    {"UnfocusPet","API/AdoptAPI/UnfocusPet"},
    {"RidePet","API/AdoptAPI/RidePet"},
    {"FlyPet","API/AdoptAPI/FlyPet"},
    {"HoldBaby","API/AdoptAPI/HoldBaby"},
    {"EjectBaby","API/AdoptAPI/EjectBaby"},
    {"UseStroller","API/AdoptAPI/UseStroller"},
    {"SitOnFloat","API/AdoptAPI/SitOnFloat"},
    {"RideParent","API/AdoptAPI/RideParent"},
    {"PassiveDoor","API/AdoptAPI/SendPassiveDoorEnter"},
    {"SpawnNursery","API/AdoptAPI/SpawnNursery"},
    {"ProgressAil","API/AilmentsAPI/ProgressPetMeAilment"},
    {"ExchangeItem","API/LootBoxAPI/ExchangeItemForReward"},
    {"RecyclePets","API/PetRecyclerAPI/PetsReturned"},
    {"CollectTicket","API/PetRecyclerAPI/TicketsCollected"},
    {"IdleAddPet","API/IdleProgressionAPI/AddPet"},
    {"IdleRemPet","API/IdleProgressionAPI/RemovePet"},
    {"CreatePetObj","API/PetObjectAPI/CreatePetObject"},
    {"DropPetObj","API/PetObjectAPI/DropPetObject"},
    {"GrabPetObj","API/PetObjectAPI/GrabPetObject"},
    {"SitPetVehicle","API/VehicleAPI/SitPetInVehicle"},
    {"RequestRide","API/VehicleAPI/RequestRide"},
    {"CompleteRide","API/VehicleAPI/CompleteRide"},
    {"SetForTrade","API/PlaceableToolAPI/SetItemForTrade"},
    {"SetPetName","API/SettingsAPI/SetPetRoleplayName"},
    {"GetPetOutfit","API/AvatarAPI/GetPetMannequinOutfitData"},
    {"SavePetOutfit","API/AvatarAPI/SavePetMannequinOutfit"},
    {"GetTradeHouse","API/HousingAPI/GetTradeHouseListings"},
    {"GetNonTradeableFurn","API/HousingAPI/GetNonTradeableFurniture"},
    {"CompleteAchieve","API/AchievementsAPI/CompleteAchievement"},
    {"FamilyInvite","API/FamilyAPI/AcceptDeclineInvite"},
    {"GetLiveOps","API/LocationAPI/GetLiveOpsMapSwapTransitionCFrame"},
    {"GetToken","API/TokenizerAPI/GetToken"},
}

local api_ok, api_miss = 0, 0
for _, v in ipairs(api_defs) do
    API[v[1]] = R(v[2])
    if API[v[1]] then api_ok = api_ok + 1 else api_miss = api_miss + 1 end
end

------------------------------------------------------------------------
-- STATE
------------------------------------------------------------------------
local S = {
    target = nil, delay = 0.3,
    spy_on = false, spy_log = {},
    dupe_count = 0, dupe_success = 0,
    farm_on = false, pet_esp = false, plr_esp = false, item_esp = false,
    noclip = false, inf_jump = false, speed = 16,
    auto_accept = false, replay_seq = {}, recording = false,
    anti_afk = true, auto_dupe_on = false, auto_dupe_method = 1,
    follow_target = false, tp_history = {}, pet_inventory = {},
    trade_log = {}, remote_cache = {}, spy_filter = "all",
    stats_start = tick(),
}

------------------------------------------------------------------------
-- GUI
------------------------------------------------------------------------
pcall(function() lp.PlayerGui:FindFirstChild("ASTRA"):Destroy() end)

local gui = Instance.new("ScreenGui")
gui.Name = "ASTRA"
gui.ResetOnSpawn = false
gui.Parent = lp.PlayerGui

local C = {
    bg = Color3.fromRGB(10, 10, 18),
    card = Color3.fromRGB(25, 25, 40),
    card2 = Color3.fromRGB(35, 35, 55),
    accent = Color3.fromRGB(140, 60, 255),
    accent2 = Color3.fromRGB(255, 45, 85),
    green = Color3.fromRGB(40, 210, 100),
    red = Color3.fromRGB(230, 45, 55),
    yellow = Color3.fromRGB(255, 210, 40),
    blue = Color3.fromRGB(50, 140, 255),
    orange = Color3.fromRGB(255, 140, 30),
    pink = Color3.fromRGB(255, 100, 180),
    cyan = Color3.fromRGB(40, 220, 220),
    text = Color3.new(1, 1, 1),
    dim = Color3.fromRGB(100, 100, 120),
    logtext = Color3.fromRGB(130, 245, 130),
    legendary = Color3.fromRGB(255, 170, 0),
    mythic = Color3.fromRGB(255, 50, 255),
}

------------------------------------------------------------------------
-- MAIN FRAME + DRAG
------------------------------------------------------------------------
local main = Instance.new("Frame")
main.Size = UDim2.new(0, 310, 0, 490)
main.Position = UDim2.new(0, 5, 0.08, 0)
main.BackgroundColor3 = C.bg
main.BackgroundTransparency = 0.03
main.BorderSizePixel = 0
main.Active = true
main.Parent = gui
Instance.new("UICorner", main).CornerRadius = UDim.new(0, 12)

local mstroke = Instance.new("UIStroke", main)
mstroke.Color = C.accent
mstroke.Thickness = 1.5
mstroke.Transparency = 0.3

local dragging, dragStart, startPos
local tbar = Instance.new("Frame")
tbar.Size = UDim2.new(1, 0, 0, 34)
tbar.BackgroundColor3 = C.accent
tbar.BackgroundTransparency = 0.1
tbar.BorderSizePixel = 0
tbar.Parent = main
Instance.new("UICorner", tbar).CornerRadius = UDim.new(0, 12)

tbar.InputBegan:Connect(function(inp)
    if inp.UserInputType == Enum.UserInputType.Touch or inp.UserInputType == Enum.UserInputType.MouseButton1 then
        dragging = true; dragStart = inp.Position; startPos = main.Position
    end
end)
UIS.InputChanged:Connect(function(inp)
    if dragging and (inp.UserInputType == Enum.UserInputType.Touch or inp.UserInputType == Enum.UserInputType.MouseMovement) then
        local d = inp.Position - dragStart
        main.Position = UDim2.new(startPos.X.Scale, startPos.X.Offset + d.X, startPos.Y.Scale, startPos.Y.Offset + d.Y)
    end
end)
UIS.InputEnded:Connect(function(inp)
    if inp.UserInputType == Enum.UserInputType.Touch or inp.UserInputType == Enum.UserInputType.MouseButton1 then dragging = false end
end)

local ttl = Instance.new("TextLabel")
ttl.Size = UDim2.new(0.55, 0, 1, 0)
ttl.Position = UDim2.new(0, 10, 0, 0)
ttl.BackgroundTransparency = 1
ttl.Text = "JC-Astra v3.0"
ttl.TextColor3 = C.text
ttl.Font = Enum.Font.GothamBlack
ttl.TextSize = 14
ttl.TextXAlignment = Enum.TextXAlignment.Left
ttl.Parent = tbar

local sdot = Instance.new("Frame")
sdot.Size = UDim2.new(0, 7, 0, 7)
sdot.Position = UDim2.new(0.58, 0, 0.5, -3)
sdot.BackgroundColor3 = C.green
sdot.BorderSizePixel = 0
sdot.Parent = tbar
Instance.new("UICorner", sdot).CornerRadius = UDim.new(1, 0)

spawn(function()
    while gui.Parent do
        pcall(function()
            Tw:Create(sdot, TweenInfo.new(0.8, Enum.EasingStyle.Sine, Enum.EasingDirection.InOut), {BackgroundTransparency = 0.6}):Play()
            wait(0.8)
            Tw:Create(sdot, TweenInfo.new(0.8, Enum.EasingStyle.Sine, Enum.EasingDirection.InOut), {BackgroundTransparency = 0}):Play()
            wait(0.8)
        end)
    end
end)

local slbl = Instance.new("TextLabel")
slbl.Size = UDim2.new(0, 80, 1, 0)
slbl.Position = UDim2.new(0.58, 11, 0, 0)
slbl.BackgroundTransparency = 1
slbl.Text = "LIVE | " .. api_ok .. " API"
slbl.TextColor3 = C.green
slbl.Font = Enum.Font.GothamBold
slbl.TextSize = 9
slbl.TextXAlignment = Enum.TextXAlignment.Left
slbl.Parent = tbar

local mini = Instance.new("TextButton")
mini.Size = UDim2.new(0, 30, 0, 30)
mini.Position = UDim2.new(1, -32, 0, 2)
mini.BackgroundTransparency = 1
mini.Text = "-"
mini.TextColor3 = C.text
mini.Font = Enum.Font.GothamBold
mini.TextSize = 20
mini.ZIndex = 10
mini.Parent = tbar

------------------------------------------------------------------------
-- TOAST
------------------------------------------------------------------------
local function toast(msg, color)
    pcall(function()
        color = color or C.accent
        local t = Instance.new("Frame")
        t.Size = UDim2.new(0, 270, 0, 28)
        t.Position = UDim2.new(0.5, -135, 0, 3)
        t.BackgroundColor3 = color
        t.BackgroundTransparency = 0.1
        t.BorderSizePixel = 0
        t.ZIndex = 60
        t.Parent = gui
        Instance.new("UICorner", t).CornerRadius = UDim.new(0, 8)

        local l = Instance.new("TextLabel", t)
        l.Size = UDim2.new(1, -8, 1, 0); l.Position = UDim2.new(0, 4, 0, 0)
        l.BackgroundTransparency = 1; l.Text = msg; l.TextColor3 = C.text
        l.Font = Enum.Font.GothamBold; l.TextSize = 11; l.ZIndex = 61
        l.TextXAlignment = Enum.TextXAlignment.Left

        spawn(function()
            wait(2.2)
            pcall(function()
                Tw:Create(t, TweenInfo.new(0.4), {BackgroundTransparency = 1}):Play()
                Tw:Create(l, TweenInfo.new(0.4), {TextTransparency = 1}):Play()
                wait(0.4); t:Destroy()
            end)
        end)
    end)
end

------------------------------------------------------------------------
-- LOG SYSTEM
------------------------------------------------------------------------
local log_cont = Instance.new("Frame")
log_cont.Size = UDim2.new(1, -6, 0, 112)
log_cont.Position = UDim2.new(0, 3, 1, -115)
log_cont.BackgroundTransparency = 1
log_cont.Parent = main

local function logbtn(txt, px, w, color, cb)
    local b = Instance.new("TextButton")
    b.Size = UDim2.new(0, w, 0, 17); b.Position = UDim2.new(0, px, 0, 0)
    b.BackgroundColor3 = color; b.BorderSizePixel = 0
    b.Text = txt; b.TextColor3 = C.text; b.Font = Enum.Font.GothamBold; b.TextSize = 8
    b.Parent = log_cont
    Instance.new("UICorner", b).CornerRadius = UDim.new(0, 4)
    b.MouseButton1Click:Connect(cb)
    return b
end

local log_history = {}
local log_n = 0

local logf = Instance.new("ScrollingFrame")
logf.Size = UDim2.new(1, 0, 0, 90)
logf.Position = UDim2.new(0, 0, 0, 20)
logf.BackgroundColor3 = Color3.fromRGB(5, 5, 10)
logf.BackgroundTransparency = 0.15
logf.BorderSizePixel = 0
logf.ScrollBarThickness = 2
logf.CanvasSize = UDim2.new(0, 0, 0, 0)
logf.Parent = log_cont

local logl = Instance.new("UIListLayout", logf)
logl.Padding = UDim.new(0, 0)
logl.SortOrder = Enum.SortOrder.LayoutOrder

local function update_canvas()
    pcall(function()
        local h = logl.AbsoluteContentSize.Y
        logf.CanvasSize = UDim2.new(0, 0, 0, h + 4)
        logf.CanvasPosition = Vector2.new(0, h + 4)
    end)
end

local function log(msg, color)
    log_n = log_n + 1
    local text = tostring(msg)
    table.insert(log_history, text)
    color = color or C.logtext

    local l = Instance.new("TextButton")
    l.Size = UDim2.new(1, -2, 0, 11)
    l.BackgroundTransparency = 1
    l.Text = text; l.TextColor3 = color
    l.Font = Enum.Font.Code; l.TextSize = 8
    l.TextXAlignment = Enum.TextXAlignment.Left
    l.TextWrapped = true; l.LayoutOrder = log_n
    l.AutoButtonColor = false; l.Parent = logf

    l.MouseButton1Click:Connect(function()
        pcall(function() setclipboard(text) end)
        l.TextColor3 = C.yellow
        spawn(function() wait(0.3); pcall(function() l.TextColor3 = color end) end)
    end)

    if log_n > 80 then
        pcall(function()
            local first = logf:FindFirstChildOfClass("TextButton")
            if first then first:Destroy() end
        end)
    end
    update_canvas()
end

logbtn("Copy All", 0, 72, C.green, function()
    pcall(function() setclipboard(table.concat(log_history, "\n")) end)
    toast("Logs copied!", C.green)
end)
logbtn("Copy 30", 75, 72, C.blue, function()
    local s = math.max(1, #log_history - 29)
    local t = {}; for i = s, #log_history do t[#t+1] = log_history[i] end
    pcall(function() setclipboard(table.concat(t, "\n")) end)
    toast("Last 30 copied!", C.blue)
end)
logbtn("Clear", 150, 50, C.red, function()
    for _, c in ipairs(logf:GetChildren()) do if c:IsA("TextButton") then c:Destroy() end end
    log_history = {}; log_n = 0; logf.CanvasSize = UDim2.new(0, 0, 0, 0)
end)
logbtn("Stats", 203, 50, C.orange, function()
    local uptime = math.floor(tick() - S.stats_start)
    local m = math.floor(uptime / 60)
    log("== STATS ==", C.orange)
    log("uptime: " .. m .. "m | dupes: " .. S.dupe_count .. " | spy: " .. #S.spy_log, C.orange)
    log("api: " .. api_ok .. "/" .. (api_ok+api_miss) .. " | target: " .. (S.target and S.target.Name or "none"), C.orange)
end)
logbtn("Export", 256, 46, C.cyan, function()
    local data = {
        delay = S.delay, speed = S.speed, dupes = S.dupe_count,
        spy_count = #S.spy_log, api = api_ok .. "/" .. (api_ok+api_miss),
        log = log_history
    }
    pcall(function() setclipboard(Http:JSONEncode(data)) end)
    toast("Full export copied!", C.cyan)
end)

------------------------------------------------------------------------
-- TAB SYSTEM
------------------------------------------------------------------------
local tab_names = {"SPY","DUPE","AUTO","REC","FARM","ESP","PETS","CFG"}
local tab_colors = {C.blue, C.red, C.mythic, C.orange, C.green, C.yellow, C.pink, C.dim}
local tab_btns = {}
local tab_pages = {}

local tab_bar = Instance.new("Frame")
tab_bar.Size = UDim2.new(1, -6, 0, 22)
tab_bar.Position = UDim2.new(0, 3, 0, 36)
tab_bar.BackgroundTransparency = 1
tab_bar.Parent = main

local tw = math.floor(302 / #tab_names) - 1
for i, name in ipairs(tab_names) do
    local b = Instance.new("TextButton")
    b.Size = UDim2.new(0, tw, 0, 20)
    b.Position = UDim2.new(0, (i-1) * (tw + 1), 0, 0)
    b.BackgroundColor3 = C.card; b.BorderSizePixel = 0
    b.Text = name; b.TextColor3 = C.dim
    b.Font = Enum.Font.GothamBold; b.TextSize = 8
    b.Parent = tab_bar
    Instance.new("UICorner", b).CornerRadius = UDim.new(0, 4)

    local p = Instance.new("ScrollingFrame")
    p.Size = UDim2.new(1, -6, 0, 290)
    p.Position = UDim2.new(0, 3, 0, 60)
    p.BackgroundTransparency = 1; p.BorderSizePixel = 0
    p.ScrollBarThickness = 2; p.CanvasSize = UDim2.new(0, 0, 0, 600)
    p.Visible = false; p.Parent = main

    tab_btns[name] = b; tab_pages[name] = p

    b.MouseButton1Click:Connect(function()
        for n, pg in pairs(tab_pages) do
            pg.Visible = (n == name)
            tab_btns[n].BackgroundColor3 = (n == name) and tab_colors[i] or C.card
            tab_btns[n].TextColor3 = (n == name) and C.text or C.dim
        end
    end)
end

local function mkb(page, text, row, col, color, cb)
    local b = Instance.new("TextButton")
    b.Size = UDim2.new(0, 146, 0, 30)
    b.Position = UDim2.new(0, col * 150, 0, row * 34)
    b.BackgroundColor3 = color or C.card
    b.BorderSizePixel = 0; b.Text = text; b.TextColor3 = C.text
    b.Font = Enum.Font.GothamBold; b.TextSize = 9
    b.Parent = page
    Instance.new("UICorner", b).CornerRadius = UDim.new(0, 6)
    b.MouseButton1Click:Connect(function() pcall(cb) end)
    return b
end

local function mkbw(page, text, row, color, cb)
    local b = Instance.new("TextButton")
    b.Size = UDim2.new(0, 296, 0, 30)
    b.Position = UDim2.new(0, 0, 0, row * 34)
    b.BackgroundColor3 = color or C.card
    b.BorderSizePixel = 0; b.Text = text; b.TextColor3 = C.text
    b.Font = Enum.Font.GothamBold; b.TextSize = 9
    b.Parent = page
    Instance.new("UICorner", b).CornerRadius = UDim.new(0, 6)
    b.MouseButton1Click:Connect(function() pcall(cb) end)
    return b
end

local function mklbl(page, text, row, color)
    local l = Instance.new("TextLabel")
    l.Size = UDim2.new(0, 296, 0, 16)
    l.Position = UDim2.new(0, 0, 0, row * 34)
    l.BackgroundTransparency = 1; l.Text = text
    l.TextColor3 = color or C.dim; l.Font = Enum.Font.GothamBold
    l.TextSize = 9; l.TextXAlignment = Enum.TextXAlignment.Left
    l.Parent = page
end

------------------------------------------------------------------------
-- UTILS
------------------------------------------------------------------------
local function fire(remote, ...)
    if not remote then return false end
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

local function serverhop()
    log("SERVER HOP...", C.yellow)
    pcall(function() TPS:Teleport(game.PlaceId, lp) end)
end

local function tpto(pos)
    pcall(function() hrp.CFrame = CFrame.new(pos) end)
end

local function find_spy(name_match)
    for i = #S.spy_log, 1, -1 do
        if S.spy_log[i].name:find(name_match) then return S.spy_log[i] end
    end
    return nil
end

local function rand_delay()
    return S.delay + (math.random() * 0.08 - 0.04)
end

------------------------------------------------------------------------
-- TAB 1: SPY
------------------------------------------------------------------------
local sp = tab_pages["SPY"]
mklbl(sp, "-- NET SPY --", 0, C.blue)

local old_nc
mkb(sp, "Spy ON/OFF", 1, 0, C.blue, function()
    S.spy_on = not S.spy_on
    if S.spy_on and not old_nc then
        old_nc = hookmetamethod(game, "__namecall", function(self, ...)
            local m = getnamecallmethod()
            if S.spy_on and (m == "FireServer" or m == "InvokeServer") then
                local args = {...}
                local nm = self.Name
                local low = nm:lower()
                local dominated = low:find("trade") or low:find("add") or low:find("offer")
                    or low:find("confirm") or low:find("accept") or low:find("equip")
                    or low:find("pet") or low:find("neon") or low:find("item")
                    or low:find("give") or low:find("focus") or low:find("fusion")
                    or low:find("decline") or low:find("remove") or low:find("unequip")
                    or low:find("recycle") or low:find("exchange") or low:find("hatch")
                    or low:find("backpack") or low:find("stash") or low:find("quiz")
                    or low:find("suggest") or low:find("ride") or low:find("fly")
                    or low:find("hold") or low:find("spawn") or low:find("idle")
                    or low:find("progress") or low:find("reward") or low:find("achieve")
                    or low:find("polymorph") or low:find("transform") or low:find("paint")

                local show = (S.spy_filter == "all" and dominated)
                    or (S.spy_filter == "trade" and (low:find("trade") or low:find("offer") or low:find("confirm") or low:find("accept") or low:find("add") or low:find("give") or low:find("decline")))
                    or (S.spy_filter == "pet" and (low:find("pet") or low:find("neon") or low:find("equip") or low:find("hatch") or low:find("ride") or low:find("fly") or low:find("fusion")))

                if show then
                    local argstr = ""
                    pcall(function() argstr = Http:JSONEncode(args) end)
                    log("[S] " .. nm, C.blue)
                    log("  " .. argstr, C.dim)
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

mkb(sp, "Filter: All", 1, 1, C.card, function()
    if S.spy_filter == "all" then S.spy_filter = "trade"
    elseif S.spy_filter == "trade" then S.spy_filter = "pet"
    else S.spy_filter = "all" end
    log("filter: " .. S.spy_filter, C.blue)
end)

mkb(sp, "Show Captured", 2, 0, C.card, function()
    log("== " .. #S.spy_log .. " captured ==", C.yellow)
    for i = math.max(1, #S.spy_log - 15), #S.spy_log do
        local e = S.spy_log[i]
        if e then log(i .. ") " .. e.name .. " " .. e.argstr) end
    end
end)

mkb(sp, "Copy All Spy", 2, 1, C.green, function()
    local t = {}
    for i, e in ipairs(S.spy_log) do t[#t+1] = i .. ") " .. e.name .. " | " .. e.argstr end
    pcall(function() setclipboard(table.concat(t, "\n")) end)
    toast("Spy copied!", C.green)
end)

mkb(sp, "Clear Spy", 3, 0, C.red, function()
    S.spy_log = {}; log("spy cleared")
end)

mklbl(sp, "-- TARGET --", 4, C.accent)

local tidx = 0
mkb(sp, "Next Target", 5, 0, C.accent, function()
    local o = {}
    for _, p in ipairs(Players:GetPlayers()) do if p ~= lp then o[#o+1] = p end end
    if #o == 0 then log("no players", C.red); return end
    tidx = tidx + 1; if tidx > #o then tidx = 1 end
    S.target = o[tidx]
    log("TARGET: " .. S.target.Name .. " (" .. tidx .. "/" .. #o .. ")", C.accent)
    toast("Target: " .. S.target.Name, C.accent)
end)

mkb(sp, "All Players", 5, 1, C.card, function()
    for i, p in ipairs(Players:GetPlayers()) do
        if p ~= lp then log(i .. ") " .. p.Name .. " [" .. p.UserId .. "]") end
    end
end)

mkb(sp, "TP to Target", 6, 0, C.card, function()
    if not S.target or not S.target.Character then log("no target", C.red); return end
    local h = S.target.Character:FindFirstChild("HumanoidRootPart")
    if h then tpto(h.Position + Vector3.new(5, 0, 0)); log("tp -> " .. S.target.Name) end
end)

mkb(sp, "API Status", 6, 1, C.card, function()
    log("API: " .. api_ok .. "/" .. (api_ok+api_miss), api_ok > api_miss and C.green or C.yellow)
    for _, v in ipairs(api_defs) do
        if not API[v[1]] then log("  X " .. v[1], C.red) end
    end
end)

------------------------------------------------------------------------
-- TAB 2: DUPE
------------------------------------------------------------------------
local dp = tab_pages["DUPE"]
mklbl(dp, "-- M1: TRADE RACE --", 0, C.red)

mkb(dp, "Send Trade", 1, 0, C.red, function()
    if not S.target then log("target first", C.red); toast("No target!", C.red); return end
    fire(API.SendTrade, S.target)
    log("trade -> " .. S.target.Name, C.red)
    toast("Trade sent!", C.red)
    table.insert(S.trade_log, {time = tick(), action = "send", target = S.target.Name})
end)

mkb(dp, "Add Pet", 1, 1, C.card, function()
    local f = find_spy("AddItemToOffer")
    if f then fire(API.AddItem, unpack(f.args)); log("added (captured)", C.green)
    else log("spy + add pet manually", C.yellow) end
end)

mkb(dp, "Accept Neg", 2, 0, C.card, function()
    fire(API.AcceptNeg); log("accepted", C.green)
end)

mkb(dp, "CONFIRM+RJ", 2, 1, Color3.fromRGB(255, 20, 20), function()
    S.dupe_count = S.dupe_count + 1
    log("=== FIRE #" .. S.dupe_count .. " d:" .. string.format("%.2f", S.delay) .. " ===", C.red)
    fire(API.Confirm)
    wait(rand_delay())
    rejoin()
end)

mklbl(dp, "-- M2: DESYNC --", 3, C.blue)

mkb(dp, "Desync 30x", 4, 0, C.blue, function()
    local eq = find_spy("Equip"); local uq = find_spy("Unequip")
    for i = 1, 30 do
        if eq then fire(API.Equip, unpack(eq.args)) end
        if uq then fire(API.Unequip, unpack(uq.args)) else fire(API.UnequipAll) end
        wait(0.02)
    end
    log("30x desync done", C.green); toast("Desync!", C.blue)
end)

mkb(dp, "Desync 100x", 4, 1, C.blue, function()
    local eq = find_spy("Equip"); local uq = find_spy("Unequip")
    for i = 1, 100 do
        if eq then fire(API.Equip, unpack(eq.args)) end
        if uq then fire(API.Unequip, unpack(uq.args)) else fire(API.UnequipAll) end
        wait(0.01)
    end
    log("100x done", C.green)
end)

mklbl(dp, "-- M3/M4/M5 --", 5, C.green)

mkb(dp, "M3: Neon Dupe", 6, 0, C.green, function()
    local n = find_spy("DoNeonFusion")
    if n then fire(API.NeonFusion, unpack(n.args)); wait(rand_delay()); rejoin()
    else log("neon cave + spy first", C.yellow) end
end)

mkb(dp, "M4: DebugStash", 6, 1, C.yellow, function()
    if not API.DebugStash then log("not found", C.red); return end
    fire(API.DebugStash); fire(API.DebugStash, true); fire(API.DebugStash, lp.UserId)
    fire(API.DebugStash, "enable"); fire(API.DebugStash, lp, true)
    log("5 variants fired", C.yellow); toast("DebugStash!", C.yellow)
end)

mkb(dp, "M5: GiveItem", 7, 0, C.accent, function()
    local f = find_spy("GiveItem")
    if f then fire(API.GiveItem, unpack(f.args)); wait(rand_delay()); rejoin()
    else log("GiveItem not captured", C.yellow) end
end)

mkb(dp, "Rejoin", 7, 1, C.red, function() rejoin() end)

mkb(dp, "Unaccept Neg", 8, 0, C.card, function()
    fire(API.UnacceptNeg); log("unaccepted")
end)

mkb(dp, "Decline Trade", 8, 1, C.card, function()
    fire(API.Decline); log("declined")
end)

------------------------------------------------------------------------
-- TAB 3: AUTO
------------------------------------------------------------------------
local ap = tab_pages["AUTO"]
mklbl(ap, "-- AUTO DUPE LOOP --", 0, C.mythic)

mkb(ap, "Method: M1", 1, 0, C.card, function()
    S.auto_dupe_method = S.auto_dupe_method + 1
    if S.auto_dupe_method > 3 then S.auto_dupe_method = 1 end
    log("auto method: M" .. S.auto_dupe_method)
end)

mkbw(ap, "START AUTO DUPE", 2, C.mythic, function()
    if S.auto_dupe_on then
        S.auto_dupe_on = false
        log("AUTO DUPE STOPPED", C.red); toast("Auto Dupe OFF", C.red); return
    end
    if not S.target then log("target first", C.red); return end
    S.auto_dupe_on = true
    log("== AUTO DUPE ON (M" .. S.auto_dupe_method .. ") ==", C.mythic)
    toast("Auto Dupe ON!", C.mythic)

    spawn(function()
        while S.auto_dupe_on do
            pcall(function()
                log("[AUTO] attempt #" .. (S.dupe_count + 1), C.mythic)
                fire(API.SendTrade, S.target); log("[AUTO] trade sent", C.dim); wait(2)
                local add = find_spy("AddItemToOffer")
                if add then fire(API.AddItem, unpack(add.args)); log("[AUTO] pet added", C.dim) end
                wait(1)
                fire(API.AcceptNeg); log("[AUTO] accepted", C.dim); wait(1.5)

                if S.auto_dupe_method == 1 then
                    S.dupe_count = S.dupe_count + 1
                    fire(API.Confirm); wait(rand_delay()); rejoin(); wait(8)
                elseif S.auto_dupe_method == 2 then
                    local eq = find_spy("Equip")
                    for i = 1, 50 do
                        if eq then fire(API.Equip, unpack(eq.args)) end
                        fire(API.UnequipAll); wait(0.02)
                    end
                    S.dupe_count = S.dupe_count + 1
                    fire(API.Confirm); wait(rand_delay()); rejoin(); wait(8)
                elseif S.auto_dupe_method == 3 then
                    local n = find_spy("DoNeonFusion")
                    if n then
                        S.dupe_count = S.dupe_count + 1
                        fire(API.NeonFusion, unpack(n.args)); wait(rand_delay()); rejoin(); wait(8)
                    else
                        log("[AUTO] neon args missing", C.red); S.auto_dupe_on = false
                    end
                end
            end)
            if S.auto_dupe_on then
                log("[AUTO] cycle done, dupes: " .. S.dupe_count, C.mythic); wait(3)
            end
        end
    end)
end)

mkbw(ap, "STOP AUTO DUPE", 3, C.red, function()
    S.auto_dupe_on = false; log("AUTO DUPE STOPPED", C.red); toast("Stopped!", C.red)
end)

mklbl(ap, "-- QUICK ACTIONS --", 4, C.dim)

mkb(ap, "Quiz Bypass", 5, 0, C.yellow, function()
    fire(API.BeginQuiz); wait(0.3)
    fire(API.AnswerQuiz, true); fire(API.AnswerQuiz, 1); fire(API.AnswerQuiz, "yes")
    fire(API.EndQuiz); log("quiz bypass attempted", C.yellow)
end)

mkb(ap, "Spam Confirm x10", 5, 1, C.red, function()
    for i = 1, 10 do fire(API.Confirm); wait(0.05) end
    log("10x confirm spam", C.red)
end)

mkb(ap, "Backpack Access", 6, 0, C.card, function()
    fire(API.ReqBackpack); log("backpack access requested")
end)

mkb(ap, "Trade History", 6, 1, C.card, function()
    local ok2, result = pcall(function() return API.GetHistory:InvokeServer() end)
    if ok2 then pcall(function() log("history: " .. Http:JSONEncode(result), C.cyan) end)
    else log("history failed", C.red) end
end)

------------------------------------------------------------------------
-- TAB 4: REC
------------------------------------------------------------------------
local rp = tab_pages["REC"]
mklbl(rp, "-- MACRO RECORDER --", 0, C.orange)

mkb(rp, "Record ON/OFF", 1, 0, C.red, function()
    S.recording = not S.recording
    if S.recording then S.replay_seq = {}; log("REC...", C.red); toast("Recording!", C.red)
    else log("rec: " .. #S.replay_seq .. " actions", C.green); toast(#S.replay_seq .. " recorded", C.green) end
end)

mkb(rp, "Play", 1, 1, C.green, function()
    if #S.replay_seq == 0 then log("empty", C.red); return end
    log("REPLAY " .. #S.replay_seq, C.orange)
    for i, a in ipairs(S.replay_seq) do
        local remote = R(a.path:gsub("ReplicatedStorage%.", ""):gsub("%.", "/"))
        if remote then fire(remote, unpack(a.args or {})) end
        if i < #S.replay_seq then
            wait(math.min(math.max(S.replay_seq[i+1].time - a.time, 0.01), 2))
        end
    end
    log("replay done", C.green)
end)

mkb(rp, "Play + Rejoin", 2, 0, C.red, function()
    if #S.replay_seq == 0 then return end
    for i, a in ipairs(S.replay_seq) do
        local remote = R(a.path:gsub("ReplicatedStorage%.", ""):gsub("%.", "/"))
        if remote then fire(remote, unpack(a.args or {})) end
        if i < #S.replay_seq then wait(math.min(math.max(S.replay_seq[i+1].time - a.time, 0.01), 1)) end
    end
    wait(S.delay); rejoin()
end)

mkb(rp, "Play 3x Speed", 2, 1, C.blue, function()
    if #S.replay_seq == 0 then return end
    for i, a in ipairs(S.replay_seq) do
        local remote = R(a.path:gsub("ReplicatedStorage%.", ""):gsub("%.", "/"))
        if remote then fire(remote, unpack(a.args or {})) end
        if i < #S.replay_seq then wait(math.min((S.replay_seq[i+1].time - a.time) / 3, 0.3)) end
    end
    log("3x done", C.green)
end)

mkb(rp, "Spam Last x50", 3, 0, C.red, function()
    if #S.spy_log == 0 then log("no data", C.red); return end
    local last = S.spy_log[#S.spy_log]
    local remote = R(last.path:gsub("ReplicatedStorage%.", ""):gsub("%.", "/"))
    if not remote then log("not found", C.red); return end
    log("x50 " .. last.name, C.red)
    for i = 1, 50 do fire(remote, unpack(last.args or {})); wait(0.015) end
    log("done", C.green)
end)

mkb(rp, "Loop Play x5", 3, 1, C.orange, function()
    if #S.replay_seq == 0 then return end
    log("loop x5", C.orange)
    for loop = 1, 5 do
        log("  loop " .. loop .. "/5", C.dim)
        for i, a in ipairs(S.replay_seq) do
            local remote = R(a.path:gsub("ReplicatedStorage%.", ""):gsub("%.", "/"))
            if remote then fire(remote, unpack(a.args or {})) end
            if i < #S.replay_seq then wait(math.min(math.max(S.replay_seq[i+1].time - a.time, 0.01), 0.5)) end
        end
        wait(0.5)
    end
    log("5 loops done", C.green)
end)

mkb(rp, "Show Seq", 4, 0, C.card, function()
    log("== SEQ: " .. #S.replay_seq .. " ==", C.yellow)
    for i, a in ipairs(S.replay_seq) do log(i .. ") " .. a.name) end
end)

mkb(rp, "Copy Seq", 4, 1, C.green, function()
    local t = {}
    for i, a in ipairs(S.replay_seq) do
        local as = ""; pcall(function() as = Http:JSONEncode(a.args) end)
        t[#t+1] = i .. ") " .. a.name .. " | " .. as
    end
    pcall(function() setclipboard(table.concat(t, "\n")) end)
    toast("Seq copied!", C.green)
end)

mkb(rp, "Clear Seq", 5, 0, C.red, function() S.replay_seq = {}; log("cleared") end)

------------------------------------------------------------------------
-- TAB 5: FARM
------------------------------------------------------------------------
local fp = tab_pages["FARM"]
mklbl(fp, "-- AUTO FARM --", 0, C.green)

local farm_conn
mkb(fp, "Auto Farm", 1, 0, C.green, function()
    S.farm_on = not S.farm_on
    if S.farm_on then
        pcall(function() char:FindFirstChildOfClass("Humanoid").WalkSpeed = S.speed end)
        farm_conn = Run.Heartbeat:Connect(function()
            if not S.farm_on then return end
            pcall(function()
                for _,obj in ipairs(WS:GetDescendants()) do
                    if obj:IsA("ProximityPrompt") and obj.Enabled then
                        local p2 = obj.Parent
                        if p2 and p2:IsA("BasePart") and (hrp.Position - p2.Position).Magnitude < 70 then
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

mkb(fp, "Auto Age x50", 1, 1, C.card, function()
    log("aging x50...", C.green)
    spawn(function()
        for i = 1, 50 do fire(API.ProgressAil); wait(0.8) end
        log("age done")
    end)
end)

mkb(fp, "Auto Accept", 2, 0, C.accent, function()
    S.auto_accept = not S.auto_accept
    log("auto accept: " .. (S.auto_accept and "ON" or "OFF"))
    toast("AutoAccept " .. (S.auto_accept and "ON" or "OFF"), C.accent)
end)

mkb(fp, "Collect Tickets", 2, 1, C.yellow, function()
    fire(API.CollectTicket); log("tickets!"); toast("Tickets!", C.yellow)
end)

mkb(fp, "Claim Reward", 3, 0, C.orange, function()
    fire(API.ClaimReward); log("reward claimed")
end)

mkb(fp, "Complete Achieve", 3, 1, C.card, function()
    fire(API.CompleteAchieve); log("achievement")
end)

mklbl(fp, "-- SPEED --", 4, C.dim)

mkb(fp, "Speed 50", 5, 0, C.card, function()
    S.speed = 50; pcall(function() char:FindFirstChildOfClass("Humanoid").WalkSpeed = 50 end); log("50")
end)
mkb(fp, "Speed 100", 5, 1, C.card, function()
    S.speed = 100; pcall(function() char:FindFirstChildOfClass("Humanoid").WalkSpeed = 100 end); log("100")
end)
mkb(fp, "Speed 200", 6, 0, C.orange, function()
    S.speed = 200; pcall(function() char:FindFirstChildOfClass("Humanoid").WalkSpeed = 200 end); log("200")
end)
mkb(fp, "Server Hop", 6, 1, C.red, function() serverhop() end)

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
-- TAB 6: ESP
------------------------------------------------------------------------
local ep = tab_pages["ESP"]
mklbl(ep, "-- MOVEMENT --", 0, C.yellow)

local nc_conn
mkb(ep, "Noclip", 1, 0, C.card, function()
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

mkb(ep, "Inf Jump", 1, 1, C.card, function()
    S.inf_jump = not S.inf_jump; log("inf jump: " .. (S.inf_jump and "ON" or "OFF"))
end)

UIS.JumpRequest:Connect(function()
    if S.inf_jump then pcall(function() char:FindFirstChildOfClass("Humanoid"):ChangeState(Enum.HumanoidStateType.Jumping) end) end
end)

mklbl(ep, "-- ESP --", 2, C.yellow)

mkb(ep, "Pet ESP", 3, 0, C.green, function()
    S.pet_esp = not S.pet_esp; log("pet esp: " .. (S.pet_esp and "ON" or "OFF"))
end)

mkb(ep, "Player ESP", 3, 1, C.red, function()
    S.plr_esp = not S.plr_esp; log("player esp: " .. (S.plr_esp and "ON" or "OFF"))
end)

mkb(ep, "Item ESP", 4, 0, C.cyan, function()
    S.item_esp = not S.item_esp; log("item esp: " .. (S.item_esp and "ON" or "OFF"))
end)

mkb(ep, "Follow Target", 4, 1, C.accent, function()
    S.follow_target = not S.follow_target
    log("follow: " .. (S.follow_target and "ON" or "OFF"))
    if S.follow_target and S.target then
        spawn(function()
            while S.follow_target and S.target and S.target.Character do
                pcall(function()
                    local th = S.target.Character:FindFirstChild("HumanoidRootPart")
                    if th then hrp.CFrame = CFrame.new(th.Position + Vector3.new(4, 0, 4)) end
                end)
                wait(0.5)
            end
        end)
    end
end)

mklbl(ep, "-- TELEPORT --", 5, C.dim)

local locs = {
    {"Nursery", Vector3.new(-250,30,-450)},
    {"PetShop", Vector3.new(-100,30,50)},
    {"NeonCave", Vector3.new(600,30,400)},
    {"School", Vector3.new(-400,30,100)},
    {"Hospital", Vector3.new(150,30,-350)},
    {"Playground", Vector3.new(100,30,200)},
    {"Campsite", Vector3.new(350,30,-100)},
    {"HotSpring", Vector3.new(500,30,300)},
}

for i, loc in ipairs(locs) do
    local row = 6 + math.floor((i-1) / 2)
    local col = (i-1) % 2
    mkb(ep, "TP " .. loc[1], row, col, C.card, function()
        tpto(loc[2]); log("tp " .. loc[1])
    end)
end

local esp_o = {}
spawn(function()
    while wait(2) do pcall(function()
        for _,o in ipairs(esp_o) do pcall(function() o:Destroy() end) end
        esp_o = {}
        if not hrp then return end

        if S.plr_esp then
            for _,p in ipairs(Players:GetPlayers()) do
                if p ~= lp and p.Character then
                    local h = p.Character:FindFirstChild("Head")
                    if h then
                        local bb = Instance.new("BillboardGui")
                        bb.Size = UDim2.new(0, 150, 0, 26); bb.AlwaysOnTop = true
                        bb.StudsOffset = Vector3.new(0, 3, 0); bb.Parent = h
                        local l = Instance.new("TextLabel", bb)
                        l.Size = UDim2.new(1,0,1,0); l.BackgroundTransparency = 1
                        l.TextColor3 = C.red; l.TextStrokeTransparency = 0
                        l.Font = Enum.Font.GothamBold; l.TextSize = 11
                        l.Text = p.Name .. " [" .. math.floor((hrp.Position - h.Position).Magnitude) .. "]"
                        esp_o[#esp_o+1] = bb
                    end
                end
            end
        end

        if S.pet_esp then
            for _,obj in ipairs(WS:GetDescendants()) do
                if obj:IsA("Model") then
                    local nm = obj.Name:lower()
                    if nm:find("pet") or nm:find("neon") or nm:find("mega")
                        or nm:find("dragon") or nm:find("unicorn") or nm:find("frost")
                        or nm:find("shadow") or nm:find("giraffe") or nm:find("parrot")
                        or nm:find("owl") or nm:find("crow") or nm:find("bat") then
                        local r = obj:FindFirstChild("HumanoidRootPart") or obj:FindFirstChildOfClass("BasePart")
                        if r then
                            local bb = Instance.new("BillboardGui")
                            bb.Size = UDim2.new(0, 130, 0, 22); bb.AlwaysOnTop = true
                            bb.StudsOffset = Vector3.new(0, 2, 0); bb.Parent = r
                            local l = Instance.new("TextLabel", bb)
                            l.Size = UDim2.new(1,0,1,0); l.BackgroundTransparency = 1
                            l.TextStrokeTransparency = 0; l.Font = Enum.Font.GothamBold; l.TextSize = 10
                            if nm:find("shadow") or nm:find("frost") or nm:find("bat") then
                                l.TextColor3 = C.mythic; l.Text = "[MYTHIC] " .. obj.Name
                            elseif nm:find("giraffe") or nm:find("owl") or nm:find("parrot") or nm:find("crow") then
                                l.TextColor3 = C.legendary; l.Text = "[LEG] " .. obj.Name
                            elseif nm:find("neon") then
                                l.TextColor3 = C.cyan; l.Text = "[NEON] " .. obj.Name
                            elseif nm:find("mega") then
                                l.TextColor3 = C.pink; l.Text = "[MEGA] " .. obj.Name
                            else
                                l.TextColor3 = C.green; l.Text = obj.Name
                            end
                            esp_o[#esp_o+1] = bb
                        end
                    end
                end
            end
        end

        if S.item_esp then
            for _,obj in ipairs(WS:GetDescendants()) do
                if obj:IsA("Tool") or (obj:IsA("Model") and obj:FindFirstChild("Handle")) then
                    local handle = obj:FindFirstChild("Handle") or obj:FindFirstChildOfClass("BasePart")
                    if handle then
                        local bb = Instance.new("BillboardGui")
                        bb.Size = UDim2.new(0, 100, 0, 20); bb.AlwaysOnTop = true
                        bb.StudsOffset = Vector3.new(0, 2, 0); bb.Parent = handle
                        local l = Instance.new("TextLabel", bb)
                        l.Size = UDim2.new(1,0,1,0); l.BackgroundTransparency = 1
                        l.TextColor3 = C.yellow; l.TextStrokeTransparency = 0
                        l.Font = Enum.Font.GothamBold; l.TextSize = 10
                        l.Text = obj.Name
                        esp_o[#esp_o+1] = bb
                    end
                end
            end
        end
    end) end
end)

------------------------------------------------------------------------
-- TAB 7: PETS
------------------------------------------------------------------------
local pp = tab_pages["PETS"]
mklbl(pp, "-- PET MANAGER --", 0, C.pink)

mkb(pp, "Scan Inventory", 1, 0, C.pink, function()
    S.pet_inventory = {}
    pcall(function()
        for _, t in ipairs(lp.Backpack:GetChildren()) do
            if t:IsA("Tool") then S.pet_inventory[#S.pet_inventory+1] = {name=t.Name, src="backpack"} end
        end
    end)
    pcall(function()
        for _, t in ipairs(char:GetChildren()) do
            if t:IsA("Tool") then S.pet_inventory[#S.pet_inventory+1] = {name=t.Name, src="equipped"} end
        end
    end)
    pcall(function()
        for _, obj in ipairs(lp.PlayerGui:GetDescendants()) do
            if obj:IsA("TextLabel") then
                local nm = obj.Name:lower()
                if (nm:find("pet") or nm:find("name")) and obj.Text ~= "" and #obj.Text < 50 then
                    S.pet_inventory[#S.pet_inventory+1] = {name=obj.Text, src="gui"}
                end
            end
        end
    end)
    pcall(function()
        for _, obj in ipairs(WS:GetDescendants()) do
            if obj:IsA("Model") then
                local owner = obj:FindFirstChild("Owner") or obj:FindFirstChild("OwnerId")
                if owner and owner:IsA("ValueBase") then
                    if owner.Value == lp.Name or owner.Value == lp.UserId then
                        S.pet_inventory[#S.pet_inventory+1] = {name=obj.Name, src="world"}
                    end
                end
            end
        end
    end)
    log("== " .. #S.pet_inventory .. " pets ==", C.pink)
    for i, p in ipairs(S.pet_inventory) do
        log(i .. ") " .. p.name .. " (" .. p.src .. ")")
    end
end)

mkb(pp, "Copy Pet List", 1, 1, C.green, function()
    local t = {}
    for i, p in ipairs(S.pet_inventory) do t[#t+1] = i .. ") " .. p.name .. " (" .. p.src .. ")" end
    pcall(function() setclipboard(table.concat(t, "\n")) end)
    toast("Pet list copied!", C.green)
end)

mkb(pp, "Ride Pet", 2, 0, C.card, function()
    local f = find_spy("RidePet")
    if f then fire(API.RidePet, unpack(f.args)) else fire(API.RidePet) end; log("ride!")
end)

mkb(pp, "Fly Pet", 2, 1, C.card, function()
    local f = find_spy("FlyPet")
    if f then fire(API.FlyPet, unpack(f.args)) else fire(API.FlyPet) end; log("fly!")
end)

mkb(pp, "Focus Pet", 3, 0, C.card, function() fire(API.FocusPet); log("focused") end)
mkb(pp, "Unfocus Pet", 3, 1, C.card, function() fire(API.UnfocusPet); log("unfocused") end)

mkb(pp, "Equip Leash", 4, 0, C.card, function() fire(API.EquipLeash); log("leash on") end)
mkb(pp, "Unequip Leash", 4, 1, C.card, function() fire(API.UnequipLeash); log("leash off") end)

mkb(pp, "Polymorph", 5, 0, C.accent, function() fire(API.Polymorph); log("polymorph!") end)
mkb(pp, "Transform", 5, 1, C.accent, function() fire(API.Transform); log("transform!") end)

mkb(pp, "Paint Pet", 6, 0, C.orange, function()
    local f = find_spy("PaintPet")
    if f then fire(API.PaintPet, unpack(f.args)) else fire(API.PaintPet) end; log("paint!")
end)

mkb(pp, "Seal Paint", 6, 1, C.orange, function() fire(API.SealPaint); log("sealed") end)

mkb(pp, "Pet Trick Up", 7, 0, C.card, function() fire(API.PetTrick); log("trick+") end)
mkb(pp, "Pet Petted", 7, 1, C.card, function() fire(API.PetPetted); log("petted!") end)

mkb(pp, "Spawn Nursery", 8, 0, C.card, function() fire(API.SpawnNursery); log("nursery spawned") end)
mkb(pp, "Mark Hatched", 8, 1, C.card, function() fire(API.MarkHatched); log("hatched!") end)

------------------------------------------------------------------------
-- TAB 8: CFG
------------------------------------------------------------------------
local cfp = tab_pages["CFG"]
mklbl(cfp, "-- REJOIN DELAY --", 0, C.dim)

mkb(cfp, "0.02s ULTRA", 1, 0, C.red, function() S.delay = 0.02; log("ULTRA", C.red); toast("ULTRA!", C.red) end)
mkb(cfp, "0.05s Aggro", 1, 1, C.orange, function() S.delay = 0.05; log("0.05s") end)
mkb(cfp, "0.1s Fast", 2, 0, C.yellow, function() S.delay = 0.1; log("0.1s") end)
mkb(cfp, "0.3s Normal", 2, 1, C.green, function() S.delay = 0.3; log("0.3s") end)
mkb(cfp, "0.5s Safe", 3, 0, C.blue, function() S.delay = 0.5; log("0.5s") end)
mkb(cfp, "1.0s Ultra Safe", 3, 1, C.card, function() S.delay = 1.0; log("1.0s") end)

mkb(cfp, "Delay -0.05", 4, 0, C.card, function()
    S.delay = math.max(0.01, S.delay - 0.05)
    log("delay: " .. string.format("%.2f", S.delay))
end)

mkb(cfp, "Delay +0.05", 4, 1, C.card, function()
    S.delay = math.min(5, S.delay + 0.05)
    log("delay: " .. string.format("%.2f", S.delay))
end)

mklbl(cfp, "-- SYSTEM --", 5, C.dim)

mkb(cfp, "Anti-AFK", 6, 0, C.green, function()
    S.anti_afk = not S.anti_afk
    log("anti-afk: " .. (S.anti_afk and "ON" or "OFF"))
end)

mkb(cfp, "Export All Data", 6, 1, C.cyan, function()
    local data = {
        delay = S.delay, speed = S.speed, dupes = S.dupe_count,
        spy = #S.spy_log, replay = #S.replay_seq,
        api = api_ok .. "/" .. (api_ok+api_miss),
        pets = #S.pet_inventory, trades = #S.trade_log,
        log = log_history, spy_data = {}
    }
    for _, e in ipairs(S.spy_log) do
        data.spy_data[#data.spy_data+1] = {name=e.name, args=e.argstr}
    end
    pcall(function() setclipboard(Http:JSONEncode(data)) end)
    toast("Full data exported!", C.cyan)
end)

mkb(cfp, "Remote Browser", 7, 0, C.card, function()
    log("== REMOTE BROWSER ==", C.yellow)
    local cnt = 0
    for _, obj in ipairs(RS:GetDescendants()) do
        if (obj:IsA("RemoteEvent") or obj:IsA("RemoteFunction")) and obj:GetFullName():find("API/") then
            cnt = cnt + 1
            if cnt <= 30 then log("  " .. obj:GetFullName()) end
        end
    end
    log("total API remotes: " .. cnt)
end)

mkb(cfp, "Close GUI", 7, 1, C.red, function() gui:Destroy() end)

mkb(cfp, "Rejoin", 8, 0, C.red, function() rejoin() end)
mkb(cfp, "Server Hop", 8, 1, C.yellow, function() serverhop() end)

spawn(function()
    while wait(50) do
        if S.anti_afk then
            pcall(function()
                local vu = game:GetService("VirtualUser")
                vu:CaptureController(); vu:ClickButton2(Vector2.new())
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
    tab_bar.Visible = not is_mini
    log_cont.Visible = not is_mini
    if not is_mini then
        for n, b in pairs(tab_btns) do
            if b.TextColor3 == C.text then tab_pages[n].Visible = true; break end
        end
    end
    main.Size = is_mini and UDim2.new(0, 310, 0, 34) or UDim2.new(0, 310, 0, 490)
    mini.Text = is_mini and "+" or "-"
end)

------------------------------------------------------------------------
-- INIT
------------------------------------------------------------------------
tab_btns["SPY"].BackgroundColor3 = C.blue
tab_btns["SPY"].TextColor3 = C.text
tab_pages["SPY"].Visible = true

log("JC-Astra v3.0 LEGENDARY", C.legendary)
log("API: " .. api_ok .. "/" .. (api_ok+api_miss) .. " | 8 tabs | 80+ features", C.green)
log("SPY > DUPE > AUTO > REC > FARM > ESP > PETS > CFG", C.dim)
toast("JC-Astra v3.0 loaded!", C.legendary)

end)

if not ok_main then
    pcall(function()
        local eg = Instance.new("ScreenGui")
        eg.Name = "ASTRA_ERR"; eg.Parent = game:GetService("Players").LocalPlayer.PlayerGui
        local ef = Instance.new("TextLabel")
        ef.Size = UDim2.new(0, 300, 0, 120); ef.Position = UDim2.new(0.5, -150, 0.35, 0)
        ef.BackgroundColor3 = Color3.fromRGB(40, 5, 5); ef.BorderSizePixel = 0
        ef.Text = "ASTRA ERROR:\n" .. tostring(err_main); ef.TextColor3 = Color3.fromRGB(255, 80, 80)
        ef.Font = Enum.Font.Code; ef.TextSize = 10; ef.TextWrapped = true; ef.Parent = eg
    end)
end
