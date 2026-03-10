local TouchDataEncoding = require("common.data.TouchDataEncoding")
---@class PlayerStack
local PlayerStack = require("client.src.PlayerStack")
local KeyDataEncoding = require("common.data.KeyDataEncoding")
local galvanized = require("common.engine.computerPlayers.galvanized.dist.galvanized")

local touchIdleInput = TouchDataEncoding.touchDataToLatinString(false, 0, 0, 6)
function PlayerStack.idleInput(self)
  return (self.inputMethod == "touch" and touchIdleInput) or KeyDataEncoding.base64encode[1]
end

function PlayerStack:send_controls()
  if self.is_local and GAME.netClient:isConnected() and #self.engine.confirmedInput > 0 and self.garbageTarget and #self.garbageTarget.engine.confirmedInput == 0 then
    -- Send 1 frame at clock time 0 then wait till we get our first input from the other player.
    -- This will cause a player that got the start message earlier than the other player to wait for the other player just once.
    --print("self.confirmedInput="..(self.confirmedInput or "nil"))
    --print("send_controls returned immediately")
    return
  end
  if self.lastDisplacementSeen ~= self.engine.displacement then
    -- print("displacement changed to "..self.engine.displacement)
    self.lastDisplacementSeen = self.engine.displacement
  end
  local input, taunt = 'A', 0
  -- countdown and first frame are not good times to send inputs
  if not self.engine.do_countdown then
    input, taunt = galvanized.getInput(self.engine)
  end
  --if input ~= 'A' then
    --print("galvanized input=")
    --print(input)
    --print("(end galvanized output)")
  --end
  GAME.netClient:sendInput(input)
  if taunt == 1 then
    self.taunt_up = math.random(#self.character.sounds.taunt_up.sources)
    GAME.netClient:sendTauntUp(self.taunt_up)
  elseif taunt == 2 then
    self.taunt_down = math.random(#self.character.sounds.taunt_down.sources)
    GAME.netClient:sendTauntDown(self.taunt_down)
  end
  self.engine:receiveConfirmedInput(input)
end
