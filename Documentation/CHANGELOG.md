

# Changelog
v0.2
- Added tinyxml2
- Refactored FileManager to now use tinyxml2 to read the level XML files. The goal is to slowly transition to a better serialisation system for levels and objects in general (RTTI)
- Fix release build
- Added GPU markers in the engine code 
- Major refactor of engien

v0.1 (and before versioning)
2020-01-17:
- Added D3D11 backed and device (D2D uses the swapchain to draw to)
- Added IMGUI support

30-3-2015:
- Added basic moving system
- Can double jump. 
- Created the first part of the level as an svg and implemented a camera class
31-3-2015
- Implemented cycling drawing modes
- Added graphic place holder for the character
- Basic walk jump and standing animation
- removed the double jump check.
- Added an arrow class that launches the player up in the air. added a bitmap
1-4-2015
- Added trigger to the avatar for checking the jumping.
- Added Entity class.
- Avatar is subclass of Entity.
- Put the whole level in a svg and updated the bitmap.
- Was implementing a god mode
2-4-2015
- Implemented setSpawnPosition method in the avatar class. Sets the checkpoint spot to respawn when dead.
- Implemented direction based camera movement. Test it with right arrow and the '1' and '2' key on the number bar.
5-04-2015
- Implemented basic start menu with quit and start button
- implemented pause button
- Implemented distance method for checking the distance between 2 entities
9-04-2015
- Start of implementing EnemyShooter class
20-04-2015
- Implemented the First Shooter interaction. Shoots the next bullet when the bullet is destroyed.
22-04-2015
- Analyzed reset bug. break when Physicsiscalled when a bullet hits the walls and fixed it.
- Added a Checkpoint class that functions as a respawn point
- Added options in the GameInit.txt to customise jumpheight and arrow push power.
23-04-2015
- Fixed some collision issues with the bullets of the EnemyShooter class.
- Added a rotating animation object called 'RotLight' used for background drawing and on checkpoints.
24-04-2015
- Added a rotating enemy, no sprite yet
- avatar: Added a death counter, goal of the game will now be to die as little as possible.
- avatar: Reduced avatar hitbox so it's easier to navigate through the level
- avatar: Now handles attack state correctly. Items will just get removed when colliding.
25/04/2015
- Created an Enemy parent class. 
- Added an enemy vector for keeping enemy objects
- [FIXED]Introduced new bug when you reload when you haven't moved you get a break point at the contact listeners.|fix: Forgot to delete the enemies in the array before reloading.
- Added a bitmap to the enemy and added options to make it move vertically too. Naming is wrong should be EnemyBlockMoving or something
- EnemyHorizontal now gets killed by the player if the player is in the ATTACK state
- Bug at [4712,1534] can jump through level
- Added a coin class.
- added a coin list class
- Coins can now be added using GameInit.txt
- Added Gold, Bronze and Silver coins.
- [BUG] Character still in ATTACK state when you hit the rotater with attack and bounce up.
- [BUG] 2 coins that have overlapping physicsActors will cause contact listener invalid access
- Added a beam animation class using raycast for determining where to put the animation.
- Added a spawnspeed for RotLight class
- Added a EnemyRocketLauncher class.
- Added a EnemyRocket class.
- EnemyRocket now follows the player at a fixed speed
- EnemyRocketLaunchssimple square.
- EnemyRocket now kills the player when hitting the player.
27/04/2015 - 
- RocketLauncher now shoots rockets at a certain interval.
- Every enemy has a name.
- Rockets don't despawn when respawning at a checkpoint
28-04-2015 - 
- Changed the way how enemies detect if they are attack.
- Added animation List;
- Enemies now spawn an animation when destroyed.
- Successfully interpolated the avatarTrail.
- Lower the trail opacity
- You can now switch to DevLevel when you press F6
- Changed InitializeAll and reset of the game class so they work with a filename
- Added a pause button. Press Escape to pause.
- Added CombRotLightCpBg class. A combination of CheckPointBg and the RotLight class.
- Rockets despawn on respawning
- Animations do not yet despawn after respawning
- Overlapping coins bug still not fixed.
- Fixed coin overlapping bug. [ DO NOT DELETE contactlistener in destructor of object. USE REMOVE METHODS, nullptr check in Coin.cpp destructor gives it back]
- Implemented a LevelEnd Class to go to the next level.
 - EnemyShooter -> Delete bullet in the tick not in begin contact
 - Fixed EnemyShooter deletion of bullets
 
 30-04-2015
 - Level Loading system
 - Drawn new level
 - Updated graphics
 - Arrows now get deleted when hit
 - New animation added for destroying enemies and rockets and bullets
 - Death counter reset every level?
 1-05-2015
 - Added a mute button.
 - Changed sound MAnager to have sounds and music in seperate vectors. 
 - changed sound manager so it has vectors to store audio levels when muting things to go back to the original level
 - Changed ui font to KenVector Future
 - Implemented Pause menu
 - Fixed the PhysicsActor Contact Listener crashing. Apparently CollisionFiltering between same objects is a great way to fix this.
 2-05-2015
 - Implemented Blockslide
 - Finished Pause menu
 - Added some level
 - Added a laser with raycasting
 - Added arrowShooters that shoot arrows
 5-4-2015
 - Updated the gameEngine begin and end contact method
 - Updated the physicsActor destructor
 - Changed the camera movement system
 - Added TriggerList class
 - Added CameraTrigger
 - Added CameraTriggerRotate
 5-5-2015
 - Finetuned level a bit
 - Fixed camera resetting
 - Changed setSpeed it now sets the desired speed
 - Fixed a bug where you could get into a infinite respawn loop
 10-5-2015
 - Added magnetic field ( MetalFan) can be aimed at a certain direction
 - Fixed magnetic field problem of not stopping
 20-5-2015
 - Fixed avatar state problem not going to sliding jump state when sliding on a platform
 - Added screenshake
 22-5-2015
 - Refactored the camera movement method.
 - Implemented Fade out and fade in for sounds. Use them with FadeIn() and FadeOut() they have to be ticked everytime though. 
 (hint to self: What about making a tick method for the sound manager?)
 - Rockets are now drawn above the level again.
1-6-2015
 - Added a keybind menu
 - Rewrote code for loading of a cfg
 - Note to self: Next game initialize avatar outside the game class( Game class is the level)
 - Levellist now loads of config.txt. Add a level by specifying the level.txt in a slot. WARNING: The cycle must count. No interupptions
 - Load keybinds from CFG, Now when pressing a key when the textbox of the keybind option menu is armed it will change the keybind to that key.
 - Create NpcHinter class. This is a little npc that dances and gives a hint when you come near it.
 2-6-2015
 - Refactored FileManager CreateObjects method. Split every type into his own private method.
 - Changed camera speed in level1
 - Added a way to read last sessions your played.