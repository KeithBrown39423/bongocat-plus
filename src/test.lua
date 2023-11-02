--==========================--
-- Just testing lua support --
--==========================--

-- laad our images. LOAD BEFORE CREATING THE WINDOW!
local up = BongoSprite.loadFromFile("cats/osu/up.png")
local left = BongoSprite.loadFromFile("cats/osu/left.png")
local right = BongoSprite.loadFromFile("cats/osu/right.png")
local wave = BongoSprite.loadFromFile("cats/osu/wave.png")
local smoke = BongoSprite.loadFromFile("cats/osu/smoke.png")
local mousebg = BongoSprite.loadFromFile("cats/osu/mousebg.png")
local testbg = BongoSprite.loadFromFile("cats/osu/temp.png")

-- states
local smoke_state = false

-- create a window
BongoWindow.create(612, 354)

while BongoWindow.processEvents() == 0 do
	-- clear the window with a color (optional but recommended)
	BongoWindow.clear(Sfml.Color.White)

	-- smoke logic
	if on_pressed(BongoInput.Key.s) then
		smoke_state = not smoke_state
	end

	-- draw the sprite to the window
	BongoWindow.draw(mousebg)

	-- draw smoke if smoke_state is true
	BongoWindow.drawif(smoke, smoke_state)

	-- display the window's contents
	BongoWindow.display()
end
