-- ╔══════════════════════════════════════════════════════════════════════╗
-- ║  JC-EXTRA-3 | MAXIMUM PAYLOAD | AdoptMe Full Domination Suite      ║
-- ║  Delta Mobile | JailCrash | 120+ Features | 90+ API Resolves       ║
-- ╚══════════════════════════════════════════════════════════════════════╝

local ok_main, err_main = pcall(function()

local Players = game:GetService("Players")
local RS = game:GetService("ReplicatedStorage")
local WS = game:GetService("Workspace")
local Http = game:GetService("HttpService")
local Run = game:GetService("RunService")
local Tw = game:GetService("TweenService")
local UIS = game:GetService("UserInputService")
local TPS = game:GetService("TeleportService")
local Light = game:GetService("Lighting")

local lp = Players.LocalPlayer
local char = lp.Character or lp.CharacterAdded:Wait()
local hrp = char:WaitForChild("HumanoidRootPart")

lp.CharacterAdded:Connect(function(c)
    char = c; hrp = c:WaitForChild("HumanoidRootPart")
end)

------------------------------------------------------------------------
-- API RESOLVE (90+)
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
    -- TradeAPI (22)
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
    -- PetAPI (16)
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
    {"SetPetAge","API/PetAPI/SetPetAge"},
    -- PetPaintAPI (4)
    {"PaintPet","API/PetPaintAPI/PaintPet"},
    {"SealPaint","API/PetPaintAPI/SealPetPaint"},
    {"BuyPaint","API/PetPaintAPI/BuyPetPaint"},
    {"WipeHairSpray","API/PetPaintAPI/WipeColoredHairSpray"},
    -- ToolAPI (3)
    {"Equip","API/ToolAPI/Equip"},
    {"Unequip","API/ToolAPI/Unequip"},
    {"UnequipAll","API/ToolAPI/UnequipAll"},
    -- ErrorReportAPI (2)
    {"DebugStash","API/ErrorReportAPI/TradeDebugStashInfiniteConfirmation"},
    {"SendError","API/ErrorReportAPI/SendUniqueError"},
    -- AdoptAPI (10)
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
    -- AilmentsAPI (1)
    {"ProgressAil","API/AilmentsAPI/ProgressPetMeAilment"},
    -- LootBoxAPI (1)
    {"ExchangeItem","API/LootBoxAPI/ExchangeItemForReward"},
    -- RecyclerAPI (2)
    {"RecyclePets","API/PetRecyclerAPI/PetsReturned"},
    {"CollectTicket","API/PetRecyclerAPI/TicketsCollected"},
    -- IdleProgressionAPI (2)
    {"IdleAddPet","API/IdleProgressionAPI/AddPet"},
    {"IdleRemPet","API/IdleProgressionAPI/RemovePet"},
    -- PetObjectAPI (3)
    {"CreatePetObj","API/PetObjectAPI/CreatePetObject"},
    {"DropPetObj","API/PetObjectAPI/DropPetObject"},
    {"GrabPetObj","API/PetObjectAPI/GrabPetObject"},
    -- VehicleAPI (3)
    {"SitPetVehicle","API/VehicleAPI/SitPetInVehicle"},
    {"RequestRide","API/VehicleAPI/RequestRide"},
    {"CompleteRide","API/VehicleAPI/CompleteRide"},
    -- PlaceableToolAPI (1)
    {"SetForTrade","API/PlaceableToolAPI/SetItemForTrade"},
    -- SettingsAPI (1)
    {"SetPetName","API/SettingsAPI/SetPetRoleplayName"},
    -- AvatarAPI (2)
    {"GetPetOutfit","API/AvatarAPI/GetPetMannequinOutfitData"},
    {"SavePetOutfit","API/AvatarAPI/SavePetMannequinOutfit"},
    -- HousingAPI (2)
    {"GetTradeHouse","API/HousingAPI/GetTradeHouseListings"},
    {"GetNonTradeableFurn","API/HousingAPI/GetNonTradeableFurniture"},
    -- AchievementsAPI (1)
    {"CompleteAchieve","API/AchievementsAPI/CompleteAchievement"},
    -- FamilyAPI (1)
    {"FamilyInvite","API/FamilyAPI/AcceptDeclineInvite"},
    -- LocationAPI (1)
    {"GetLiveOps","API/LocationAPI/GetLiveOpsMapSwapTransitionCFrame"},
    -- TokenizerAPI (1)
    {"GetToken","API/TokenizerAPI/GetToken"},
    -- EggAPI (extras)
    {"BuyEgg","API/EggAPI/BuyEgg"},
    {"HatchEgg","API/EggAPI/HatchEgg"},
    {"OpenEgg","API/EggAPI/OpenEgg"},
    -- GiftAPI
    {"ClaimGift","API/GiftAPI/ClaimGift"},
    {"OpenGift","API/GiftAPI/OpenGift"},
    -- DailyRewardAPI
    {"ClaimDaily","API/DailyRewardAPI/ClaimReward"},
    {"ClaimLogin","API/LoginRewardAPI/ClaimReward"},
    -- CurrencyAPI
    {"GetBucks","API/CurrencyAPI/GetBucks"},
    {"SpendBucks","API/CurrencyAPI/SpendBucks"},
    -- BabyAPI
    {"FeedBaby","API/BabyAPI/FeedBaby"},
    {"SleepBaby","API/BabyAPI/SleepBaby"},
    {"CleanBaby","API/BabyAPI/CleanBaby"},
    {"BathBaby","API/BabyAPI/BathBaby"},
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
    target = nil, delay = 0.3, spy_on = false, spy_log = {},
    dupe_count = 0, dupe_success = 0, farm_on = false,
    pet_esp = false, plr_esp = false, item_esp = false,
    noclip = false, inf_jump = false, speed = 16, fly_on = false,
    auto_accept = false, replay_seq = {}, recording = false,
    anti_afk = true, auto_dupe_on = false, auto_dupe_method = 1,
    follow_target = false, pet_inventory = {}, trade_log = {},
    spy_filter = "all", stats_start = tick(),
    fullbright = false, freecam = false, low_grav = false,
    auto_hatch = false, auto_care = false, auto_collect = false,
    dupe_queue = {}, dupe_q_running = false,
    anti_kick = true, fly_speed = 50,
}

------------------------------------------------------------------------
-- GUI SETUP
------------------------------------------------------------------------
pcall(function() lp.PlayerGui:FindFirstChild("EXTRA3"):Destroy() end)

local gui = Instance.new("ScreenGui")
gui.Name = "EXTRA3"; gui.ResetOnSpawn = false; gui.Parent = lp.PlayerGui

local C = {
    bg = Color3.fromRGB(8, 8, 16), card = Color3.fromRGB(22, 22, 38),
    card2 = Color3.fromRGB(32, 32, 52), accent = Color3.fromRGB(130, 50, 255),
    accent2 = Color3.fromRGB(255, 40, 80), green = Color3.fromRGB(35, 210, 95),
    red = Color3.fromRGB(230, 40, 50), yellow = Color3.fromRGB(255, 210, 35),
    blue = Color3.fromRGB(45, 135, 255), orange = Color3.fromRGB(255, 135, 25),
    pink = Color3.fromRGB(255, 95, 175), cyan = Color3.fromRGB(35, 220, 220),
    text = Color3.new(1, 1, 1), dim = Color3.fromRGB(90, 90, 110),
    logtext = Color3.fromRGB(120, 240, 120), legendary = Color3.fromRGB(255, 165, 0),
    mythic = Color3.fromRGB(255, 45, 255), gold = Color3.fromRGB(255, 215, 0),
    lime = Color3.fromRGB(100, 255, 50), crimson = Color3.fromRGB(180, 20, 40),
}

------------------------------------------------------------------------
-- MAIN FRAME
------------------------------------------------------------------------
local main = Instance.new("Frame")
main.Size = UDim2.new(0, 320, 0, 510)
main.Position = UDim2.new(0, 4, 0.06, 0)
main.BackgroundColor3 = C.bg; main.BackgroundTransparency = 0.02
main.BorderSizePixel = 0; main.Active = true; main.Parent = gui
Instance.new("UICorner", main).CornerRadius = UDim.new(0, 14)

local ms = Instance.new("UIStroke", main)
ms.Color = C.accent; ms.Thickness = 1.6; ms.Transparency = 0.2

-- drag
local dragging, dragStart, startPos
local tbar = Instance.new("Frame")
tbar.Size = UDim2.new(1, 0, 0, 36); tbar.BackgroundColor3 = C.accent
tbar.BackgroundTransparency = 0.08; tbar.BorderSizePixel = 0; tbar.Parent = main
Instance.new("UICorner", tbar).CornerRadius = UDim.new(0, 14)

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

-- title
local ttl = Instance.new("TextLabel")
ttl.Size = UDim2.new(0.5, 0, 1, 0); ttl.Position = UDim2.new(0, 10, 0, 0)
ttl.BackgroundTransparency = 1; ttl.Text = "JC-EXTRA-3"
ttl.TextColor3 = C.gold; ttl.Font = Enum.Font.GothamBlack
ttl.TextSize = 15; ttl.TextXAlignment = Enum.TextXAlignment.Left; ttl.Parent = tbar

-- status dot + pulse
local sdot = Instance.new("Frame")
sdot.Size = UDim2.new(0, 8, 0, 8); sdot.Position = UDim2.new(0.52, 0, 0.5, -4)
sdot.BackgroundColor3 = C.green; sdot.BorderSizePixel = 0; sdot.Parent = tbar
Instance.new("UICorner", sdot).CornerRadius = UDim.new(1, 0)

spawn(function()
    while gui.Parent do
        pcall(function()
            Tw:Create(sdot, TweenInfo.new(0.7, Enum.EasingStyle.Sine, Enum.EasingDirection.InOut), {BackgroundTransparency = 0.65}):Play(); wait(0.7)
            Tw:Create(sdot, TweenInfo.new(0.7, Enum.EasingStyle.Sine, Enum.EasingDirection.InOut), {BackgroundTransparency = 0}):Play(); wait(0.7)
        end)
    end
end)

local slbl = Instance.new("TextLabel")
slbl.Size = UDim2.new(0, 90, 1, 0); slbl.Position = UDim2.new(0.52, 12, 0, 0)
slbl.BackgroundTransparency = 1; slbl.Text = api_ok .. " API | EXTRA"
slbl.TextColor3 = C.green; slbl.Font = Enum.Font.GothamBold
slbl.TextSize = 9; slbl.TextXAlignment = Enum.TextXAlignment.Left; slbl.Parent = tbar

local mini = Instance.new("TextButton")
mini.Size = UDim2.new(0, 32, 0, 32); mini.Position = UDim2.new(1, -34, 0, 2)
mini.BackgroundTransparency = 1; mini.Text = "-"
mini.TextColor3 = C.text; mini.Font = Enum.Font.GothamBold
mini.TextSize = 22; mini.ZIndex = 10; mini.Parent = tbar

------------------------------------------------------------------------
-- TOAST
------------------------------------------------------------------------
local function toast(msg, color)
    pcall(function()
        color = color or C.accent
        local t = Instance.new("Frame")
        t.Size = UDim2.new(0, 280, 0, 28); t.Position = UDim2.new(0.5, -140, 0, 2)
        t.BackgroundColor3 = color; t.BackgroundTransparency = 0.08
        t.BorderSizePixel = 0; t.ZIndex = 60; t.Parent = gui
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
                wait(0.5); t:Destroy()
            end)
        end)
    end)
end

------------------------------------------------------------------------
-- LOG
------------------------------------------------------------------------
local log_cont = Instance.new("Frame")
log_cont.Size = UDim2.new(1, -6, 0, 112)
log_cont.Position = UDim2.new(0, 3, 1, -115)
log_cont.BackgroundTransparency = 1; log_cont.Parent = main

local function logbtn(txt, px, w, color, cb)
    local b = Instance.new("TextButton")
    b.Size = UDim2.new(0, w, 0, 17); b.Position = UDim2.new(0, px, 0, 0)
    b.BackgroundColor3 = color; b.BorderSizePixel = 0
    b.Text = txt; b.TextColor3 = C.text; b.Font = Enum.Font.GothamBold; b.TextSize = 8
    b.Parent = log_cont; Instance.new("UICorner", b).CornerRadius = UDim.new(0, 4)
    b.MouseButton1Click:Connect(cb); return b
end

local log_history, log_n = {}, 0
local logf = Instance.new("ScrollingFrame")
logf.Size = UDim2.new(1, 0, 0, 90); logf.Position = UDim2.new(0, 0, 0, 20)
logf.BackgroundColor3 = Color3.fromRGB(4, 4, 10); logf.BackgroundTransparency = 0.12
logf.BorderSizePixel = 0; logf.ScrollBarThickness = 2
logf.CanvasSize = UDim2.new(0, 0, 0, 0); logf.Parent = log_cont
local logl = Instance.new("UIListLayout", logf)
logl.Padding = UDim.new(0, 0); logl.SortOrder = Enum.SortOrder.LayoutOrder

local function update_canvas()
    pcall(function()
        local h = logl.AbsoluteContentSize.Y
        logf.CanvasSize = UDim2.new(0, 0, 0, h + 4)
        logf.CanvasPosition = Vector2.new(0, h + 4)
    end)
end

local function log(msg, color)
    log_n = log_n + 1
    local text = tostring(msg); table.insert(log_history, text)
    color = color or C.logtext
    local l = Instance.new("TextButton")
    l.Size = UDim2.new(1, -2, 0, 11); l.BackgroundTransparency = 1
    l.Text = text; l.TextColor3 = color; l.Font = Enum.Font.Code; l.TextSize = 8
    l.TextXAlignment = Enum.TextXAlignment.Left; l.TextWrapped = true
    l.LayoutOrder = log_n; l.AutoButtonColor = false; l.Parent = logf
    l.MouseButton1Click:Connect(function()
        pcall(function() setclipboard(text) end)
        l.TextColor3 = C.yellow
        spawn(function() wait(0.3); pcall(function() l.TextColor3 = color end) end)
    end)
    if log_n > 100 then
        pcall(function() local f = logf:FindFirstChildOfClass("TextButton"); if f then f:Destroy() end end)
    end
    update_canvas()
end

logbtn("CopyAll", 0, 55, C.green, function()
    pcall(function() setclipboard(table.concat(log_history, "\n")) end); toast("Copied!", C.green)
end)
logbtn("Copy30", 58, 52, C.blue, function()
    local s = math.max(1, #log_history - 29); local t = {}
    for i = s, #log_history do t[#t+1] = log_history[i] end
    pcall(function() setclipboard(table.concat(t, "\n")) end); toast("30 copied", C.blue)
end)
logbtn("Clear", 113, 40, C.red, function()
    for _, c in ipairs(logf:GetChildren()) do if c:IsA("TextButton") then c:Destroy() end end
    log_history = {}; log_n = 0; logf.CanvasSize = UDim2.new(0, 0, 0, 0)
end)
logbtn("Stats", 156, 42, C.orange, function()
    local m = math.floor((tick() - S.stats_start) / 60)
    log("== STATS ==", C.orange)
    log("uptime:" .. m .. "m dupe:" .. S.dupe_count .. " spy:" .. #S.spy_log .. " q:" .. #S.dupe_queue, C.orange)
    log("api:" .. api_ok .. "/" .. (api_ok+api_miss) .. " tgt:" .. (S.target and S.target.Name or "-"), C.orange)
end)
logbtn("Export", 201, 46, C.cyan, function()
    local d = {delay=S.delay,speed=S.speed,dupes=S.dupe_count,spy=#S.spy_log,
        replay=#S.replay_seq,api=api_ok.."/".. (api_ok+api_miss),pets=#S.pet_inventory,
        log=log_history,spy_data={}}
    for _, e in ipairs(S.spy_log) do d.spy_data[#d.spy_data+1] = {name=e.name,args=e.argstr} end
    pcall(function() setclipboard(Http:JSONEncode(d)) end); toast("Exported!", C.cyan)
end)
logbtn("JSON", 250, 40, C.dim, function()
    pcall(function() setclipboard(Http:JSONEncode({d=S.delay,s=S.speed,dc=S.dupe_count,
        sl=#S.spy_log,ao=api_ok,am=api_miss})) end); toast("JSON!", C.dim)
end)

------------------------------------------------------------------------
-- TABS
------------------------------------------------------------------------
local tab_names = {"SPY","DUPE","AUTO","REC","FARM","ESP","PETS","CFG"}
local tab_colors = {C.blue,C.red,C.mythic,C.orange,C.green,C.yellow,C.pink,C.dim}
local tab_btns, tab_pages = {}, {}

local tab_bar = Instance.new("Frame")
tab_bar.Size = UDim2.new(1, -6, 0, 22); tab_bar.Position = UDim2.new(0, 3, 0, 38)
tab_bar.BackgroundTransparency = 1; tab_bar.Parent = main

local tw2 = math.floor(312 / #tab_names) - 1
for i, name in ipairs(tab_names) do
    local b = Instance.new("TextButton")
    b.Size = UDim2.new(0, tw2, 0, 20); b.Position = UDim2.new(0, (i-1) * (tw2 + 1), 0, 0)
    b.BackgroundColor3 = C.card; b.BorderSizePixel = 0; b.Text = name
    b.TextColor3 = C.dim; b.Font = Enum.Font.GothamBold; b.TextSize = 8
    b.Parent = tab_bar; Instance.new("UICorner", b).CornerRadius = UDim.new(0, 4)
    local p = Instance.new("ScrollingFrame")
    p.Size = UDim2.new(1, -6, 0, 300); p.Position = UDim2.new(0, 3, 0, 62)
    p.BackgroundTransparency = 1; p.BorderSizePixel = 0; p.ScrollBarThickness = 2
    p.CanvasSize = UDim2.new(0, 0, 0, 800); p.Visible = false; p.Parent = main
    tab_btns[name] = b; tab_pages[name] = p
    b.MouseButton1Click:Connect(function()
        for n, pg in pairs(tab_pages) do
            pg.Visible = (n == name)
            tab_btns[n].BackgroundColor3 = (n == name) and tab_colors[i] or C.card
            tab_btns[n].TextColor3 = (n == name) and C.text or C.dim
        end
    end)
end

-- factories
local function mkb(pg, txt, row, col, clr, cb)
    local b = Instance.new("TextButton")
    b.Size = UDim2.new(0, 150, 0, 28); b.Position = UDim2.new(0, col * 155, 0, row * 32)
    b.BackgroundColor3 = clr or C.card; b.BorderSizePixel = 0; b.Text = txt
    b.TextColor3 = C.text; b.Font = Enum.Font.GothamBold; b.TextSize = 9
    b.Parent = pg; Instance.new("UICorner", b).CornerRadius = UDim.new(0, 6)
    b.MouseButton1Click:Connect(function() pcall(cb) end); return b
end
local function mkbw(pg, txt, row, clr, cb)
    local b = Instance.new("TextButton")
    b.Size = UDim2.new(0, 306, 0, 28); b.Position = UDim2.new(0, 0, 0, row * 32)
    b.BackgroundColor3 = clr or C.card; b.BorderSizePixel = 0; b.Text = txt
    b.TextColor3 = C.text; b.Font = Enum.Font.GothamBold; b.TextSize = 9
    b.Parent = pg; Instance.new("UICorner", b).CornerRadius = UDim.new(0, 6)
    b.MouseButton1Click:Connect(function() pcall(cb) end); return b
end
local function mklbl(pg, txt, row, clr)
    local l = Instance.new("TextLabel")
    l.Size = UDim2.new(0, 306, 0, 14); l.Position = UDim2.new(0, 0, 0, row * 32)
    l.BackgroundTransparency = 1; l.Text = txt; l.TextColor3 = clr or C.dim
    l.Font = Enum.Font.GothamBold; l.TextSize = 9
    l.TextXAlignment = Enum.TextXAlignment.Left; l.Parent = pg
end

------------------------------------------------------------------------
-- UTILS
------------------------------------------------------------------------
local function fire(remote, ...)
    if not remote then return false end
    local ok2, err2
    if remote:IsA("RemoteEvent") then ok2, err2 = pcall(function() remote:FireServer(...) end)
    else ok2, err2 = pcall(function() remote:InvokeServer(...) end) end
    if not ok2 then log("ERR:" .. tostring(err2), C.red) end; return ok2
end
local function rejoin() log("REJOIN...", C.yellow); pcall(function() TPS:TeleportToPlaceInstance(game.PlaceId, game.JobId, lp) end) end
local function serverhop() log("HOP...", C.yellow); pcall(function() TPS:Teleport(game.PlaceId, lp) end) end
local function tpto(pos) pcall(function() hrp.CFrame = CFrame.new(pos) end) end
local function find_spy(nm)
    for i = #S.spy_log, 1, -1 do if S.spy_log[i].name:find(nm) then return S.spy_log[i] end end
end
local function rd() return S.delay + (math.random() * 0.08 - 0.04) end

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
                local args = {...}; local nm = self.Name; local low = nm:lower()
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
                    or low:find("egg") or low:find("gift") or low:find("daily")
                    or low:find("feed") or low:find("sleep") or low:find("clean")
                    or low:find("currency") or low:find("bucks") or low:find("buy")
                local show = (S.spy_filter == "all" and dominated)
                    or (S.spy_filter == "trade" and (low:find("trade") or low:find("offer") or low:find("confirm") or low:find("accept") or low:find("add") or low:find("give") or low:find("decline")))
                    or (S.spy_filter == "pet" and (low:find("pet") or low:find("neon") or low:find("equip") or low:find("hatch") or low:find("ride") or low:find("fly") or low:find("fusion") or low:find("egg")))
                if show then
                    local argstr = ""; pcall(function() argstr = Http:JSONEncode(args) end)
                    log("[S] " .. nm, C.blue); log("  " .. argstr, C.dim)
                    table.insert(S.spy_log, {name=nm, path=self:GetFullName(), args=args, argstr=argstr, time=tick()})
                    if S.recording then table.insert(S.replay_seq, {name=nm, path=self:GetFullName(), args=args, time=tick()}) end
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
    else S.spy_filter = "all" end; log("filter:" .. S.spy_filter, C.blue)
end)

mkb(sp, "Show Captured", 2, 0, C.card, function()
    log("== " .. #S.spy_log .. " captured ==", C.yellow)
    for i = math.max(1, #S.spy_log - 20), #S.spy_log do
        local e = S.spy_log[i]; if e then log(i .. ") " .. e.name .. " " .. e.argstr) end
    end
end)

mkb(sp, "Copy All Spy", 2, 1, C.green, function()
    local t = {}; for i, e in ipairs(S.spy_log) do t[#t+1] = i .. ") " .. e.name .. " | " .. e.argstr end
    pcall(function() setclipboard(table.concat(t, "\n")) end); toast("Spy copied!", C.green)
end)

mkb(sp, "Clear Spy", 3, 0, C.red, function() S.spy_log = {}; log("spy cleared") end)

mklbl(sp, "-- TARGET --", 4, C.accent)
local tidx = 0
mkb(sp, "Next Target", 5, 0, C.accent, function()
    local o = {}; for _, p in ipairs(Players:GetPlayers()) do if p ~= lp then o[#o+1] = p end end
    if #o == 0 then log("no players", C.red); return end
    tidx = tidx + 1; if tidx > #o then tidx = 1 end
    S.target = o[tidx]; log("TARGET: " .. S.target.Name .. " (" .. tidx .. "/" .. #o .. ")", C.accent)
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
    log("API:" .. api_ok .. "/" .. (api_ok+api_miss), api_ok > api_miss and C.green or C.yellow)
    for _, v in ipairs(api_defs) do if not API[v[1]] then log("  X " .. v[1], C.red) end end
end)

mklbl(sp, "-- REMOTE TOOLS --", 7, C.dim)

mkb(sp, "Remote Browser", 8, 0, C.card, function()
    local cnt = 0
    for _, obj in ipairs(RS:GetDescendants()) do
        if (obj:IsA("RemoteEvent") or obj:IsA("RemoteFunction")) and obj:GetFullName():find("API/") then
            cnt = cnt + 1; if cnt <= 40 then log("  " .. obj:GetFullName()) end
        end
    end; log("total: " .. cnt, C.yellow)
end)

mkb(sp, "Fuzz Last", 8, 1, C.orange, function()
    if #S.spy_log == 0 then log("nothing", C.red); return end
    local last = S.spy_log[#S.spy_log]
    local remote = R(last.path:gsub("ReplicatedStorage%.", ""):gsub("%.", "/"))
    if not remote then log("gone", C.red); return end
    log("FUZZ: " .. last.name, C.orange)
    pcall(function() fire(remote) end)
    pcall(function() fire(remote, true) end)
    pcall(function() fire(remote, false) end)
    pcall(function() fire(remote, 1) end)
    pcall(function() fire(remote, lp) end)
    pcall(function() fire(remote, lp.UserId) end)
    pcall(function() fire(remote, "") end)
    pcall(function() fire(remote, nil, true) end)
    if last.args and #last.args > 0 then
        pcall(function() fire(remote, unpack(last.args)) end)
        pcall(function() fire(remote, last.args[1], true) end)
    end
    log("10 variants fired", C.green)
end)

mkb(sp, "World Scan", 9, 0, C.cyan, function()
    log("== WORLD SCAN ==", C.cyan)
    local pets, items, proms = 0, 0, 0
    for _, obj in ipairs(WS:GetDescendants()) do
        if obj:IsA("Model") and (obj.Name:lower():find("pet") or obj:FindFirstChild("HumanoidRootPart")) then pets = pets + 1 end
        if obj:IsA("Tool") or (obj:IsA("Model") and obj:FindFirstChild("Handle")) then items = items + 1 end
        if obj:IsA("ProximityPrompt") then proms = proms + 1 end
    end
    log("pets:" .. pets .. " items:" .. items .. " prompts:" .. proms, C.cyan)
    log("players:" .. #Players:GetPlayers() .. " jobId:" .. game.JobId:sub(1, 12), C.cyan)
end)

mkb(sp, "Server Info", 9, 1, C.card, function()
    log("== SERVER ==", C.yellow)
    log("place:" .. game.PlaceId .. " job:" .. game.JobId:sub(1, 16), C.yellow)
    log("players:" .. #Players:GetPlayers() .. "/" .. Players.MaxPlayers, C.yellow)
    log("fps:" .. math.floor(1/Run.Heartbeat:Wait()), C.yellow)
end)

------------------------------------------------------------------------
-- TAB 2: DUPE (7 methods + queue)
------------------------------------------------------------------------
local dp = tab_pages["DUPE"]
mklbl(dp, "-- M1: TRADE RACE --", 0, C.red)

mkb(dp, "Send Trade", 1, 0, C.red, function()
    if not S.target then log("target!", C.red); toast("No target!", C.red); return end
    fire(API.SendTrade, S.target); log("trade -> " .. S.target.Name, C.red)
    toast("Trade sent!", C.red)
end)

mkb(dp, "Add Pet", 1, 1, C.card, function()
    local f = find_spy("AddItemToOffer")
    if f then fire(API.AddItem, unpack(f.args)); log("added", C.green)
    else log("spy + add pet first", C.yellow) end
end)

mkb(dp, "Accept Neg", 2, 0, C.card, function() fire(API.AcceptNeg); log("accepted", C.green) end)

mkb(dp, "CONFIRM+RJ", 2, 1, Color3.fromRGB(255, 15, 15), function()
    S.dupe_count = S.dupe_count + 1
    log("=== FIRE #" .. S.dupe_count .. " d:" .. string.format("%.2f", S.delay) .. " ===", C.red)
    fire(API.Confirm); wait(rd()); rejoin()
end)

mklbl(dp, "-- M2: DESYNC --", 3, C.blue)

mkb(dp, "Desync 30x", 4, 0, C.blue, function()
    local eq = find_spy("Equip"); local uq = find_spy("Unequip")
    for i = 1, 30 do
        if eq then fire(API.Equip, unpack(eq.args)) end
        if uq then fire(API.Unequip, unpack(uq.args)) else fire(API.UnequipAll) end; wait(0.02)
    end; log("30x done", C.green); toast("Desync!", C.blue)
end)

mkb(dp, "Desync 100x", 4, 1, C.blue, function()
    local eq = find_spy("Equip"); local uq = find_spy("Unequip")
    for i = 1, 100 do
        if eq then fire(API.Equip, unpack(eq.args)) end
        if uq then fire(API.Unequip, unpack(uq.args)) else fire(API.UnequipAll) end; wait(0.01)
    end; log("100x done", C.green)
end)

mklbl(dp, "-- M3-M7 --", 5, C.green)

mkb(dp, "M3: Neon", 6, 0, C.green, function()
    local n = find_spy("DoNeonFusion")
    if n then fire(API.NeonFusion, unpack(n.args)); wait(rd()); rejoin()
    else log("neon cave + spy", C.yellow) end
end)

mkb(dp, "M4: DebugStash", 6, 1, C.yellow, function()
    if not API.DebugStash then log("X", C.red); return end
    fire(API.DebugStash); fire(API.DebugStash, true); fire(API.DebugStash, lp.UserId)
    fire(API.DebugStash, "enable"); fire(API.DebugStash, lp, true)
    log("5 variants", C.yellow); toast("DebugStash!", C.yellow)
end)

mkb(dp, "M5: GiveItem", 7, 0, C.accent, function()
    local f = find_spy("GiveItem")
    if f then fire(API.GiveItem, unpack(f.args)); wait(rd()); rejoin()
    else log("GiveItem uncaptured", C.yellow) end
end)

mkb(dp, "M6: Recycle", 7, 1, C.orange, function()
    local f = find_spy("PetsReturned")
    if f then fire(API.RecyclePets, unpack(f.args)); wait(rd()); rejoin()
    else fire(API.RecyclePets); fire(API.RecyclePets, true); fire(API.RecyclePets, lp)
        log("recycle 3 variants", C.orange) end
end)

mkb(dp, "M7: IdleProg", 8, 0, C.cyan, function()
    local f = find_spy("AddPet")
    if f then fire(API.IdleAddPet, unpack(f.args)); wait(0.1)
        fire(API.IdleRemPet, unpack(f.args)); wait(rd()); rejoin()
    else fire(API.IdleAddPet); fire(API.IdleRemPet); log("idle 2 variants", C.cyan) end
end)

mkb(dp, "Rejoin", 8, 1, C.red, function() rejoin() end)

mklbl(dp, "-- TRADE TOOLS --", 9, C.dim)

mkb(dp, "Unaccept", 10, 0, C.card, function() fire(API.UnacceptNeg); log("unaccepted") end)
mkb(dp, "Decline", 10, 1, C.card, function() fire(API.Decline); log("declined") end)
mkb(dp, "RemoveItem", 11, 0, C.card, function()
    local f = find_spy("RemoveItemFromOffer")
    if f then fire(API.RemoveItem, unpack(f.args)); log("removed") else log("uncaptured", C.yellow) end
end)
mkb(dp, "SuggestItem", 11, 1, C.card, function()
    local f = find_spy("SuggestItem")
    if f then fire(API.SuggestItem, unpack(f.args)); log("suggested") else log("uncaptured", C.yellow) end
end)

mklbl(dp, "-- DUPE QUEUE --", 12, C.mythic)

mkb(dp, "Q: Add Last", 13, 0, C.mythic, function()
    local f = find_spy("AddItemToOffer")
    if f then table.insert(S.dupe_queue, {args=f.args, name=f.name})
        log("queued #" .. #S.dupe_queue, C.mythic); toast("Queued!", C.mythic)
    else log("capture add first", C.yellow) end
end)

mkb(dp, "Q: Show", 13, 1, C.card, function()
    log("== QUEUE: " .. #S.dupe_queue .. " ==", C.mythic)
    for i, q in ipairs(S.dupe_queue) do log(i .. ") " .. q.name) end
end)

mkbw(dp, "Q: RUN ALL (M1)", 14, C.crimson, function()
    if S.dupe_q_running then S.dupe_q_running = false; log("Q STOP", C.red); return end
    if #S.dupe_queue == 0 then log("empty", C.red); return end
    if not S.target then log("target!", C.red); return end
    S.dupe_q_running = true; log("== Q RUN " .. #S.dupe_queue .. " ==", C.mythic)
    spawn(function()
        for qi, q in ipairs(S.dupe_queue) do
            if not S.dupe_q_running then break end
            log("[Q " .. qi .. "/" .. #S.dupe_queue .. "]", C.mythic)
            fire(API.SendTrade, S.target); wait(2)
            fire(API.AddItem, unpack(q.args)); wait(1)
            fire(API.AcceptNeg); wait(1.5)
            S.dupe_count = S.dupe_count + 1
            fire(API.Confirm); wait(rd()); rejoin(); wait(10)
        end
        S.dupe_q_running = false; log("Q DONE", C.green)
    end)
end)

mkb(dp, "Q: Clear", 15, 0, C.red, function() S.dupe_queue = {}; log("q cleared") end)

------------------------------------------------------------------------
-- TAB 3: AUTO
------------------------------------------------------------------------
local ap = tab_pages["AUTO"]
mklbl(ap, "-- AUTO DUPE LOOP --", 0, C.mythic)

mkb(ap, "Method: M1", 1, 0, C.card, function()
    S.auto_dupe_method = S.auto_dupe_method + 1
    if S.auto_dupe_method > 5 then S.auto_dupe_method = 1 end
    log("method: M" .. S.auto_dupe_method)
end)

mkbw(ap, "START AUTO DUPE", 2, C.mythic, function()
    if S.auto_dupe_on then S.auto_dupe_on = false; log("STOP", C.red); toast("Off", C.red); return end
    if not S.target then log("target!", C.red); return end
    S.auto_dupe_on = true; log("== AUTO M" .. S.auto_dupe_method .. " ==", C.mythic)
    toast("Auto ON!", C.mythic)
    spawn(function()
        while S.auto_dupe_on do pcall(function()
            log("[A] #" .. (S.dupe_count+1), C.mythic)
            fire(API.SendTrade, S.target); wait(2)
            local add = find_spy("AddItemToOffer")
            if add then fire(API.AddItem, unpack(add.args)) end; wait(1)
            fire(API.AcceptNeg); wait(1.5)
            if S.auto_dupe_method == 1 then
                S.dupe_count = S.dupe_count + 1; fire(API.Confirm); wait(rd()); rejoin(); wait(8)
            elseif S.auto_dupe_method == 2 then
                local eq = find_spy("Equip")
                for i = 1, 50 do if eq then fire(API.Equip, unpack(eq.args)) end; fire(API.UnequipAll); wait(0.02) end
                S.dupe_count = S.dupe_count + 1; fire(API.Confirm); wait(rd()); rejoin(); wait(8)
            elseif S.auto_dupe_method == 3 then
                local n = find_spy("DoNeonFusion")
                if n then S.dupe_count = S.dupe_count + 1; fire(API.NeonFusion, unpack(n.args)); wait(rd()); rejoin(); wait(8)
                else log("neon X", C.red); S.auto_dupe_on = false end
            elseif S.auto_dupe_method == 4 then
                fire(API.DebugStash); fire(API.DebugStash, true); fire(API.DebugStash, lp.UserId)
                S.dupe_count = S.dupe_count + 1; fire(API.Confirm); wait(rd()); rejoin(); wait(8)
            elseif S.auto_dupe_method == 5 then
                local f = find_spy("GiveItem")
                if f then S.dupe_count = S.dupe_count + 1; fire(API.GiveItem, unpack(f.args)); wait(rd()); rejoin(); wait(8)
                else log("give X", C.red); S.auto_dupe_on = false end
            end
        end); if S.auto_dupe_on then log("[A] cycle:" .. S.dupe_count, C.mythic); wait(3) end end
    end)
end)

mkbw(ap, "STOP AUTO DUPE", 3, C.red, function()
    S.auto_dupe_on = false; log("STOPPED", C.red); toast("Stopped!", C.red)
end)

mklbl(ap, "-- AUTO HATCH --", 4, C.lime)

mkb(ap, "Auto Hatch", 5, 0, C.lime, function()
    S.auto_hatch = not S.auto_hatch; log("hatch:" .. (S.auto_hatch and "ON" or "OFF"), C.lime)
    toast("Hatch " .. (S.auto_hatch and "ON" or "OFF"), C.lime)
    if S.auto_hatch then
        spawn(function()
            while S.auto_hatch do pcall(function()
                fire(API.BuyEgg); wait(0.5); fire(API.HatchEgg); wait(0.5)
                fire(API.OpenEgg); fire(API.MarkHatched); wait(2)
            end) end
        end)
    end
end)

mkb(ap, "Auto Care", 5, 1, C.pink, function()
    S.auto_care = not S.auto_care; log("care:" .. (S.auto_care and "ON" or "OFF"), C.pink)
    toast("Care " .. (S.auto_care and "ON" or "OFF"), C.pink)
    if S.auto_care then
        spawn(function()
            while S.auto_care do pcall(function()
                fire(API.FeedBaby); wait(1); fire(API.SleepBaby); wait(1)
                fire(API.CleanBaby); wait(1); fire(API.BathBaby); wait(1)
                fire(API.ProgressAil); wait(2)
            end) end
        end)
    end
end)

mklbl(ap, "-- QUICK --", 6, C.dim)

mkb(ap, "Quiz Bypass", 7, 0, C.yellow, function()
    fire(API.BeginQuiz); wait(0.3)
    fire(API.AnswerQuiz, true); fire(API.AnswerQuiz, 1); fire(API.AnswerQuiz, "yes")
    fire(API.EndQuiz); log("quiz bypass", C.yellow)
end)

mkb(ap, "Spam Cfm x20", 7, 1, C.red, function()
    for i = 1, 20 do fire(API.Confirm); wait(0.03) end; log("20x confirm", C.red)
end)

mkb(ap, "Backpack", 8, 0, C.card, function() fire(API.ReqBackpack); log("backpack") end)

mkb(ap, "Trade History", 8, 1, C.card, function()
    local ok2, r = pcall(function() return API.GetHistory:InvokeServer() end)
    if ok2 then pcall(function() log("hist:" .. Http:JSONEncode(r), C.cyan) end)
    else log("fail", C.red) end
end)

mkb(ap, "Auto Collect", 9, 0, C.gold, function()
    S.auto_collect = not S.auto_collect; log("collect:" .. (S.auto_collect and "ON" or "OFF"), C.gold)
    if S.auto_collect then spawn(function()
        while S.auto_collect do pcall(function()
            fire(API.ClaimGift); fire(API.OpenGift); fire(API.ClaimDaily)
            fire(API.ClaimLogin); fire(API.ClaimReward); fire(API.CollectTicket)
            fire(API.CompleteAchieve); fire(API.ExchangeItem); wait(10)
        end) end
    end) end
end)

mkb(ap, "Spam GiveItem", 9, 1, C.red, function()
    local f = find_spy("GiveItem")
    if f then for i = 1, 10 do fire(API.GiveItem, unpack(f.args)); wait(0.05) end; log("10x give", C.red)
    else log("uncaptured", C.yellow) end
end)

------------------------------------------------------------------------
-- TAB 4: REC
------------------------------------------------------------------------
local rp = tab_pages["REC"]
mklbl(rp, "-- MACRO RECORDER --", 0, C.orange)

mkb(rp, "Record ON/OFF", 1, 0, C.red, function()
    S.recording = not S.recording
    if S.recording then S.replay_seq = {}; log("REC...", C.red); toast("Recording!", C.red)
    else log("rec:" .. #S.replay_seq, C.green); toast(#S.replay_seq .. " recorded", C.green) end
end)

mkb(rp, "Play", 1, 1, C.green, function()
    if #S.replay_seq == 0 then log("empty", C.red); return end
    log("REPLAY " .. #S.replay_seq, C.orange)
    for i, a in ipairs(S.replay_seq) do
        local remote = R(a.path:gsub("ReplicatedStorage%.", ""):gsub("%.", "/"))
        if remote then fire(remote, unpack(a.args or {})) end
        if i < #S.replay_seq then wait(math.min(math.max(S.replay_seq[i+1].time - a.time, 0.01), 2)) end
    end; log("done", C.green)
end)

mkb(rp, "Play+Rejoin", 2, 0, C.red, function()
    if #S.replay_seq == 0 then return end
    for i, a in ipairs(S.replay_seq) do
        local remote = R(a.path:gsub("ReplicatedStorage%.", ""):gsub("%.", "/"))
        if remote then fire(remote, unpack(a.args or {})) end
        if i < #S.replay_seq then wait(math.min(math.max(S.replay_seq[i+1].time - a.time, 0.01), 1)) end
    end; wait(S.delay); rejoin()
end)

mkb(rp, "Play 3x", 2, 1, C.blue, function()
    if #S.replay_seq == 0 then return end
    for i, a in ipairs(S.replay_seq) do
        local remote = R(a.path:gsub("ReplicatedStorage%.", ""):gsub("%.", "/"))
        if remote then fire(remote, unpack(a.args or {})) end
        if i < #S.replay_seq then wait(math.min((S.replay_seq[i+1].time - a.time) / 3, 0.3)) end
    end; log("3x done", C.green)
end)

mkb(rp, "Spam Last x50", 3, 0, C.red, function()
    if #S.spy_log == 0 then log("empty", C.red); return end
    local last = S.spy_log[#S.spy_log]
    local remote = R(last.path:gsub("ReplicatedStorage%.", ""):gsub("%.", "/"))
    if not remote then log("gone", C.red); return end
    log("x50 " .. last.name, C.red)
    for i = 1, 50 do fire(remote, unpack(last.args or {})); wait(0.015) end; log("done", C.green)
end)

mkb(rp, "Spam x200", 3, 1, C.crimson, function()
    if #S.spy_log == 0 then log("empty", C.red); return end
    local last = S.spy_log[#S.spy_log]
    local remote = R(last.path:gsub("ReplicatedStorage%.", ""):gsub("%.", "/"))
    if not remote then return end; log("x200 " .. last.name, C.crimson)
    for i = 1, 200 do fire(remote, unpack(last.args or {})); wait(0.008) end; log("done", C.green)
end)

mkb(rp, "Loop x5", 4, 0, C.orange, function()
    if #S.replay_seq == 0 then return end; log("loop x5", C.orange)
    for loop = 1, 5 do log("  " .. loop .. "/5", C.dim)
        for i, a in ipairs(S.replay_seq) do
            local remote = R(a.path:gsub("ReplicatedStorage%.", ""):gsub("%.", "/"))
            if remote then fire(remote, unpack(a.args or {})) end
            if i < #S.replay_seq then wait(math.min(math.max(S.replay_seq[i+1].time - a.time, 0.01), 0.5)) end
        end; wait(0.5) end; log("done", C.green)
end)

mkb(rp, "Loop x20", 4, 1, C.crimson, function()
    if #S.replay_seq == 0 then return end; log("loop x20", C.crimson)
    for loop = 1, 20 do
        for i, a in ipairs(S.replay_seq) do
            local remote = R(a.path:gsub("ReplicatedStorage%.", ""):gsub("%.", "/"))
            if remote then fire(remote, unpack(a.args or {})) end
            if i < #S.replay_seq then wait(math.min((S.replay_seq[i+1].time - a.time) / 2, 0.3)) end
        end; wait(0.3) end; log("20 loops done", C.green)
end)

mkb(rp, "Show Seq", 5, 0, C.card, function()
    log("== SEQ:" .. #S.replay_seq .. " ==", C.yellow)
    for i, a in ipairs(S.replay_seq) do log(i .. ") " .. a.name) end
end)

mkb(rp, "Copy Seq", 5, 1, C.green, function()
    local t = {}; for i, a in ipairs(S.replay_seq) do
        local as = ""; pcall(function() as = Http:JSONEncode(a.args) end)
        t[#t+1] = i .. ") " .. a.name .. " | " .. as end
    pcall(function() setclipboard(table.concat(t, "\n")) end); toast("Copied!", C.green)
end)

mkb(rp, "Clear Seq", 6, 0, C.red, function() S.replay_seq = {}; log("cleared") end)

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
    else if farm_conn then farm_conn:Disconnect(); farm_conn = nil end end
    log("farm:" .. (S.farm_on and "ON" or "OFF")); toast("Farm " .. (S.farm_on and "ON" or "OFF"), C.green)
end)

mkb(fp, "Age x100", 1, 1, C.card, function()
    log("aging x100", C.green); spawn(function()
        for i = 1, 100 do fire(API.ProgressAil); wait(0.6) end; log("done") end)
end)

mkb(fp, "Auto Accept", 2, 0, C.accent, function()
    S.auto_accept = not S.auto_accept; log("accept:" .. (S.auto_accept and "ON" or "OFF"))
    toast("AutoAccept " .. (S.auto_accept and "ON" or "OFF"), C.accent)
end)

mkb(fp, "Collect All", 2, 1, C.yellow, function()
    fire(API.CollectTicket); fire(API.ClaimReward); fire(API.ClaimGift)
    fire(API.ClaimDaily); fire(API.ClaimLogin); fire(API.CompleteAchieve)
    log("collected all!", C.yellow); toast("Collected!", C.yellow)
end)

mklbl(fp, "-- SPEED --", 3, C.dim)

mkb(fp, "Speed 50", 4, 0, C.card, function()
    S.speed = 50; pcall(function() char:FindFirstChildOfClass("Humanoid").WalkSpeed = 50 end); log("50")
end)
mkb(fp, "Speed 100", 4, 1, C.card, function()
    S.speed = 100; pcall(function() char:FindFirstChildOfClass("Humanoid").WalkSpeed = 100 end); log("100")
end)
mkb(fp, "Speed 200", 5, 0, C.orange, function()
    S.speed = 200; pcall(function() char:FindFirstChildOfClass("Humanoid").WalkSpeed = 200 end); log("200")
end)
mkb(fp, "Speed 500", 5, 1, C.red, function()
    S.speed = 500; pcall(function() char:FindFirstChildOfClass("Humanoid").WalkSpeed = 500 end); log("500!")
end)
mkb(fp, "JumpPower 200", 6, 0, C.card, function()
    pcall(function() char:FindFirstChildOfClass("Humanoid").JumpPower = 200 end); log("jp 200")
end)
mkb(fp, "Server Hop", 6, 1, C.red, function() serverhop() end)

pcall(function()
    lp.PlayerGui.DescendantAdded:Connect(function(obj)
        if not S.auto_accept then return end
        pcall(function() wait(0.3)
            if (obj:IsA("TextButton") or obj:IsA("ImageButton")) then
                local n = obj.Name:lower()
                if n:find("accept") or n:find("confirm") then
                    firesignal(obj.Activated); log("[AA] " .. obj.Name, C.green)
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
    if S.noclip then nc_conn = Run.Stepped:Connect(function()
        pcall(function() for _,p2 in ipairs(char:GetDescendants()) do if p2:IsA("BasePart") then p2.CanCollide = false end end end)
    end) else if nc_conn then nc_conn:Disconnect(); nc_conn = nil end end
    log("noclip:" .. (S.noclip and "ON" or "OFF"))
end)

mkb(ep, "Inf Jump", 1, 1, C.card, function()
    S.inf_jump = not S.inf_jump; log("jump:" .. (S.inf_jump and "ON" or "OFF"))
end)
UIS.JumpRequest:Connect(function()
    if S.inf_jump then pcall(function() char:FindFirstChildOfClass("Humanoid"):ChangeState(Enum.HumanoidStateType.Jumping) end) end
end)

mkb(ep, "Fly", 2, 0, C.blue, function()
    S.fly_on = not S.fly_on; log("fly:" .. (S.fly_on and "ON" or "OFF"), C.blue)
    toast("Fly " .. (S.fly_on and "ON" or "OFF"), C.blue)
    if S.fly_on then
        local bp = Instance.new("BodyVelocity"); bp.Name = "JCFly"
        bp.MaxForce = Vector3.new(math.huge, math.huge, math.huge)
        bp.Velocity = Vector3.new(0, 0, 0); bp.Parent = hrp
        local bg = Instance.new("BodyGyro"); bg.Name = "JCGyro"
        bg.MaxTorque = Vector3.new(math.huge, math.huge, math.huge); bg.Parent = hrp
        spawn(function()
            while S.fly_on and hrp do pcall(function()
                local cam = WS.CurrentCamera
                bg.CFrame = cam.CFrame
                local vel = Vector3.new(0, 0, 0)
                local mv = char:FindFirstChildOfClass("Humanoid").MoveDirection
                if mv.Magnitude > 0 then vel = cam.CFrame:VectorToWorldSpace(Vector3.new(0, 0, -S.fly_speed)) end
                if UIS:IsKeyDown(Enum.KeyCode.Space) then vel = vel + Vector3.new(0, S.fly_speed, 0) end
                bp.Velocity = vel
            end); wait(0.01) end
        end)
    else
        pcall(function() hrp:FindFirstChild("JCFly"):Destroy() end)
        pcall(function() hrp:FindFirstChild("JCGyro"):Destroy() end)
    end
end)

mkb(ep, "Fly Speed+", 2, 1, C.card, function()
    S.fly_speed = S.fly_speed + 25; log("fly:" .. S.fly_speed)
end)

mkb(ep, "Fullbright", 3, 0, C.yellow, function()
    S.fullbright = not S.fullbright
    if S.fullbright then
        pcall(function()
            Light.Ambient = Color3.new(1, 1, 1); Light.Brightness = 2
            Light.FogEnd = 1e10; Light.GlobalShadows = false
        end)
    else pcall(function()
        Light.Ambient = Color3.fromRGB(127, 127, 127); Light.Brightness = 1
        Light.GlobalShadows = true end) end
    log("bright:" .. (S.fullbright and "ON" or "OFF"))
end)

mkb(ep, "Low Gravity", 3, 1, C.card, function()
    S.low_grav = not S.low_grav
    pcall(function() WS.Gravity = S.low_grav and 40 or 196.2 end)
    log("gravity:" .. (S.low_grav and "LOW" or "NORMAL"))
end)

mklbl(ep, "-- ESP --", 4, C.yellow)

mkb(ep, "Pet ESP", 5, 0, C.green, function()
    S.pet_esp = not S.pet_esp; log("pet esp:" .. (S.pet_esp and "ON" or "OFF"))
end)
mkb(ep, "Player ESP", 5, 1, C.red, function()
    S.plr_esp = not S.plr_esp; log("plr esp:" .. (S.plr_esp and "ON" or "OFF"))
end)
mkb(ep, "Item ESP", 6, 0, C.cyan, function()
    S.item_esp = not S.item_esp; log("item esp:" .. (S.item_esp and "ON" or "OFF"))
end)
mkb(ep, "Follow Target", 6, 1, C.accent, function()
    S.follow_target = not S.follow_target; log("follow:" .. (S.follow_target and "ON" or "OFF"))
    if S.follow_target and S.target then spawn(function()
        while S.follow_target and S.target and S.target.Character do pcall(function()
            local th = S.target.Character:FindFirstChild("HumanoidRootPart")
            if th then hrp.CFrame = CFrame.new(th.Position + Vector3.new(4, 0, 4)) end
        end); wait(0.4) end
    end) end
end)

mklbl(ep, "-- TELEPORT --", 7, C.dim)
local locs = {
    {"Nursery", Vector3.new(-250,30,-450)}, {"PetShop", Vector3.new(-100,30,50)},
    {"NeonCave", Vector3.new(600,30,400)}, {"School", Vector3.new(-400,30,100)},
    {"Hospital", Vector3.new(150,30,-350)}, {"Playground", Vector3.new(100,30,200)},
    {"Campsite", Vector3.new(350,30,-100)}, {"HotSpring", Vector3.new(500,30,300)},
    {"Bridge", Vector3.new(200,30,400)}, {"SafetyHub", Vector3.new(-300,30,300)},
    {"Pool", Vector3.new(0,30,150)}, {"TreeHouse", Vector3.new(-150,30,-200)},
}
for i, loc in ipairs(locs) do
    local row = 8 + math.floor((i-1) / 2); local col = (i-1) % 2
    mkb(ep, "TP " .. loc[1], row, col, C.card, function() tpto(loc[2]); log("tp " .. loc[1]) end)
end

-- ESP loop
local esp_o = {}
spawn(function()
    while wait(2) do pcall(function()
        for _,o in ipairs(esp_o) do pcall(function() o:Destroy() end) end; esp_o = {}
        if not hrp then return end
        if S.plr_esp then for _,p in ipairs(Players:GetPlayers()) do
            if p ~= lp and p.Character then local h = p.Character:FindFirstChild("Head")
                if h then local bb = Instance.new("BillboardGui")
                    bb.Size = UDim2.new(0, 150, 0, 26); bb.AlwaysOnTop = true
                    bb.StudsOffset = Vector3.new(0, 3, 0); bb.Parent = h
                    local l = Instance.new("TextLabel", bb)
                    l.Size = UDim2.new(1,0,1,0); l.BackgroundTransparency = 1
                    l.TextColor3 = C.red; l.TextStrokeTransparency = 0
                    l.Font = Enum.Font.GothamBold; l.TextSize = 11
                    l.Text = p.Name .. " [" .. math.floor((hrp.Position - h.Position).Magnitude) .. "]"
                    esp_o[#esp_o+1] = bb end end end end
        if S.pet_esp then for _,obj in ipairs(WS:GetDescendants()) do
            if obj:IsA("Model") then local nm = obj.Name:lower()
                if nm:find("pet") or nm:find("neon") or nm:find("mega") or nm:find("dragon")
                    or nm:find("unicorn") or nm:find("frost") or nm:find("shadow")
                    or nm:find("giraffe") or nm:find("parrot") or nm:find("owl")
                    or nm:find("crow") or nm:find("bat") or nm:find("phoenix")
                    or nm:find("kitsune") or nm:find("cerberus") or nm:find("turtle") then
                    local r = obj:FindFirstChild("HumanoidRootPart") or obj:FindFirstChildOfClass("BasePart")
                    if r then local bb = Instance.new("BillboardGui")
                        bb.Size = UDim2.new(0, 130, 0, 22); bb.AlwaysOnTop = true
                        bb.StudsOffset = Vector3.new(0, 2, 0); bb.Parent = r
                        local l = Instance.new("TextLabel", bb)
                        l.Size = UDim2.new(1,0,1,0); l.BackgroundTransparency = 1
                        l.TextStrokeTransparency = 0; l.Font = Enum.Font.GothamBold; l.TextSize = 10
                        if nm:find("shadow") or nm:find("frost") or nm:find("bat") then
                            l.TextColor3 = C.mythic; l.Text = "[MYTH] " .. obj.Name
                        elseif nm:find("giraffe") or nm:find("owl") or nm:find("parrot") or nm:find("crow") or nm:find("phoenix") then
                            l.TextColor3 = C.legendary; l.Text = "[LEG] " .. obj.Name
                        elseif nm:find("mega") then l.TextColor3 = C.pink; l.Text = "[MEGA] " .. obj.Name
                        elseif nm:find("neon") then l.TextColor3 = C.cyan; l.Text = "[NEON] " .. obj.Name
                        else l.TextColor3 = C.green; l.Text = obj.Name end
                        esp_o[#esp_o+1] = bb end end end end end
        if S.item_esp then for _,obj in ipairs(WS:GetDescendants()) do
            if obj:IsA("Tool") or (obj:IsA("Model") and obj:FindFirstChild("Handle")) then
                local handle = obj:FindFirstChild("Handle") or obj:FindFirstChildOfClass("BasePart")
                if handle then local bb = Instance.new("BillboardGui")
                    bb.Size = UDim2.new(0, 100, 0, 20); bb.AlwaysOnTop = true
                    bb.StudsOffset = Vector3.new(0, 2, 0); bb.Parent = handle
                    local l = Instance.new("TextLabel", bb)
                    l.Size = UDim2.new(1,0,1,0); l.BackgroundTransparency = 1
                    l.TextColor3 = C.yellow; l.TextStrokeTransparency = 0
                    l.Font = Enum.Font.GothamBold; l.TextSize = 10; l.Text = obj.Name
                    esp_o[#esp_o+1] = bb end end end end
    end) end
end)

------------------------------------------------------------------------
-- TAB 7: PETS
------------------------------------------------------------------------
local pp = tab_pages["PETS"]
mklbl(pp, "-- PET MANAGER --", 0, C.pink)

mkb(pp, "Scan Inventory", 1, 0, C.pink, function()
    S.pet_inventory = {}
    pcall(function() for _, t in ipairs(lp.Backpack:GetChildren()) do
        if t:IsA("Tool") then S.pet_inventory[#S.pet_inventory+1] = {name=t.Name, src="bp"} end end end)
    pcall(function() for _, t in ipairs(char:GetChildren()) do
        if t:IsA("Tool") then S.pet_inventory[#S.pet_inventory+1] = {name=t.Name, src="eq"} end end end)
    pcall(function() for _, obj in ipairs(lp.PlayerGui:GetDescendants()) do
        if obj:IsA("TextLabel") then local nm = obj.Name:lower()
            if (nm:find("pet") or nm:find("name")) and obj.Text ~= "" and #obj.Text < 50 then
                S.pet_inventory[#S.pet_inventory+1] = {name=obj.Text, src="gui"} end end end end)
    pcall(function() for _, obj in ipairs(WS:GetDescendants()) do
        if obj:IsA("Model") then local owner = obj:FindFirstChild("Owner") or obj:FindFirstChild("OwnerId")
            if owner and owner:IsA("ValueBase") then
                if owner.Value == lp.Name or owner.Value == lp.UserId then
                    S.pet_inventory[#S.pet_inventory+1] = {name=obj.Name, src="world"} end end end end end)
    log("== " .. #S.pet_inventory .. " pets ==", C.pink)
    for i, p in ipairs(S.pet_inventory) do log(i .. ") " .. p.name .. " (" .. p.src .. ")") end
end)

mkb(pp, "Copy Pets", 1, 1, C.green, function()
    local t = {}; for i, p in ipairs(S.pet_inventory) do t[#t+1] = i .. ") " .. p.name .. " (" .. p.src .. ")" end
    pcall(function() setclipboard(table.concat(t, "\n")) end); toast("Copied!", C.green)
end)

mkb(pp, "Ride", 2, 0, C.card, function()
    local f = find_spy("RidePet"); if f then fire(API.RidePet, unpack(f.args)) else fire(API.RidePet) end; log("ride!")
end)
mkb(pp, "Fly", 2, 1, C.card, function()
    local f = find_spy("FlyPet"); if f then fire(API.FlyPet, unpack(f.args)) else fire(API.FlyPet) end; log("fly!")
end)
mkb(pp, "Focus", 3, 0, C.card, function() fire(API.FocusPet); log("focused") end)
mkb(pp, "Unfocus", 3, 1, C.card, function() fire(API.UnfocusPet); log("unfocused") end)
mkb(pp, "Equip Leash", 4, 0, C.card, function() fire(API.EquipLeash); log("leash on") end)
mkb(pp, "Unequip Leash", 4, 1, C.card, function() fire(API.UnequipLeash); log("leash off") end)
mkb(pp, "Polymorph", 5, 0, C.accent, function() fire(API.Polymorph); log("polymorph!") end)
mkb(pp, "Transform", 5, 1, C.accent, function() fire(API.Transform); log("transform!") end)
mkb(pp, "Paint Pet", 6, 0, C.orange, function()
    local f = find_spy("PaintPet"); if f then fire(API.PaintPet, unpack(f.args)) else fire(API.PaintPet) end; log("paint!")
end)
mkb(pp, "Seal Paint", 6, 1, C.orange, function() fire(API.SealPaint); log("sealed") end)
mkb(pp, "Trick Up", 7, 0, C.card, function() fire(API.PetTrick); log("trick+") end)
mkb(pp, "Pet Petted", 7, 1, C.card, function() fire(API.PetPetted); log("petted!") end)
mkb(pp, "Nursery", 8, 0, C.card, function() fire(API.SpawnNursery); log("nursery") end)
mkb(pp, "Hatched", 8, 1, C.card, function() fire(API.MarkHatched); log("hatched!") end)
mkb(pp, "Hold Baby", 9, 0, C.pink, function() fire(API.HoldBaby); log("holding!") end)
mkb(pp, "Eject Baby", 9, 1, C.pink, function() fire(API.EjectBaby); log("ejected!") end)
mkb(pp, "Stroller", 10, 0, C.card, function() fire(API.UseStroller); log("stroller!") end)
mkb(pp, "Sit Float", 10, 1, C.card, function() fire(API.SitOnFloat); log("floating!") end)
mkb(pp, "Rename Pet", 11, 0, C.card, function()
    fire(API.SetPetName, "JC-EXTRA"); log("renamed JC-EXTRA")
end)
mkb(pp, "Equip All", 11, 1, C.card, function()
    local f = find_spy("Equip"); if f then fire(API.Equip, unpack(f.args)); log("equipped") else log("uncapt", C.yellow) end
end)
mkb(pp, "UnequipAll", 12, 0, C.red, function() fire(API.UnequipAll); log("all unequipped") end)
mkb(pp, "Wipe Paint", 12, 1, C.red, function() fire(API.WipeHairSpray); log("wiped!") end)

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
mkb(cfp, "1.0s Ultra", 3, 1, C.card, function() S.delay = 1.0; log("1.0s") end)
mkb(cfp, "Delay -0.05", 4, 0, C.card, function()
    S.delay = math.max(0.01, S.delay - 0.05); log("d:" .. string.format("%.2f", S.delay))
end)
mkb(cfp, "Delay +0.05", 4, 1, C.card, function()
    S.delay = math.min(5, S.delay + 0.05); log("d:" .. string.format("%.2f", S.delay))
end)

mklbl(cfp, "-- SYSTEM --", 5, C.dim)

mkb(cfp, "Anti-AFK", 6, 0, C.green, function()
    S.anti_afk = not S.anti_afk; log("afk:" .. (S.anti_afk and "ON" or "OFF"))
end)
mkb(cfp, "Anti-Kick", 6, 1, C.green, function()
    S.anti_kick = not S.anti_kick; log("kick:" .. (S.anti_kick and "ON" or "OFF"))
end)
mkb(cfp, "Export Data", 7, 0, C.cyan, function()
    local d = {delay=S.delay,speed=S.speed,dupes=S.dupe_count,spy=#S.spy_log,
        replay=#S.replay_seq,api=api_ok.."/".. (api_ok+api_miss),pets=#S.pet_inventory,
        queue=#S.dupe_queue,log=log_history,spy_data={}}
    for _, e in ipairs(S.spy_log) do d.spy_data[#d.spy_data+1] = {name=e.name,args=e.argstr} end
    pcall(function() setclipboard(Http:JSONEncode(d)) end); toast("Exported!", C.cyan)
end)
mkb(cfp, "Close GUI", 7, 1, C.red, function() gui:Destroy() end)
mkb(cfp, "Rejoin", 8, 0, C.red, function() rejoin() end)
mkb(cfp, "Server Hop", 8, 1, C.yellow, function() serverhop() end)
mkb(cfp, "Reset Char", 9, 0, C.card, function()
    pcall(function() char:FindFirstChildOfClass("Humanoid").Health = 0 end); log("reset")
end)
mkb(cfp, "Copy PlaceId", 9, 1, C.card, function()
    pcall(function() setclipboard(tostring(game.PlaceId)) end); log("placeId copied")
end)
mkb(cfp, "Copy JobId", 10, 0, C.card, function()
    pcall(function() setclipboard(game.JobId) end); log("jobId copied")
end)
mkb(cfp, "Full Reset", 10, 1, C.crimson, function()
    S.spy_log = {}; S.replay_seq = {}; S.dupe_queue = {}; S.pet_inventory = {}
    S.dupe_count = 0; log_history = {}; log("FULL RESET", C.crimson)
end)

-- anti-afk + anti-kick
spawn(function()
    while wait(45) do
        if S.anti_afk then pcall(function()
            local vu = game:GetService("VirtualUser")
            vu:CaptureController(); vu:ClickButton2(Vector2.new())
        end) end
    end
end)

pcall(function()
    if S.anti_kick then
        local mt = getrawmetatable(game)
        if mt and setreadonly then
            local old_index = mt.__index
            setreadonly(mt, false)
            mt.__index = newcclosure(function(self, key)
                if key == "Kick" or key == "kick" then return function() end end
                return old_index(self, key)
            end)
            setreadonly(mt, true)
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
    tab_bar.Visible = not is_mini; log_cont.Visible = not is_mini
    if not is_mini then
        for n, b in pairs(tab_btns) do
            if b.TextColor3 == C.text then tab_pages[n].Visible = true; break end
        end
    end
    main.Size = is_mini and UDim2.new(0, 320, 0, 36) or UDim2.new(0, 320, 0, 510)
    mini.Text = is_mini and "+" or "-"
end)

------------------------------------------------------------------------
-- INIT
------------------------------------------------------------------------
tab_btns["SPY"].BackgroundColor3 = C.blue; tab_btns["SPY"].TextColor3 = C.text
tab_pages["SPY"].Visible = true

log("JC-EXTRA-3 | MAXIMUM PAYLOAD", C.gold)
log("API:" .. api_ok .. "/" .. (api_ok+api_miss) .. " | 8 tabs | 120+ features", C.green)
log("7 dupe methods | queue | auto hatch/care/collect", C.mythic)
log("fly | fullbright | low grav | 12 teleports | anti-kick", C.cyan)
toast("JC-EXTRA-3 loaded!", C.gold)

end)

if not ok_main then
    pcall(function()
        local eg = Instance.new("ScreenGui")
        eg.Name = "EXTRA3_ERR"; eg.Parent = game:GetService("Players").LocalPlayer.PlayerGui
        local ef = Instance.new("TextLabel")
        ef.Size = UDim2.new(0, 300, 0, 130); ef.Position = UDim2.new(0.5, -150, 0.3, 0)
        ef.BackgroundColor3 = Color3.fromRGB(40, 5, 5); ef.BorderSizePixel = 0
        ef.Text = "EXTRA3 ERROR:\n" .. tostring(err_main); ef.TextColor3 = Color3.fromRGB(255, 80, 80)
        ef.Font = Enum.Font.Code; ef.TextSize = 10; ef.TextWrapped = true; ef.Parent = eg
    end)
end
