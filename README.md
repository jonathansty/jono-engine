# Electronic Jona Joy

This game is a psychedelic platformer game where you live in a world 
where electronic music is the most important thing. The purpose is 
to make your way through all the levels and beating some bosses like 
the electro Pope.

This project is not an original game and serves mainly as a learning framework for C++/Game Programming. The game is built using the DAE framework with custom tweaks.

Video: https://www.youtube.com/watch?v=jKIbhicOz88 

# Feature Overview
Game Parts Description:
	+ Player:
		- Can run, jump.
		- Has an attack: Jump and press X will smash the ground and destroy enemies, rockets and other things
		- Can receive power ups like flying, superspeed, etc
		- Triggers checkpoints
		- is targeted by rockets
		- when he dies he spawns at the previous checkpoint
		- When he dies you can fast respawn with 'r' and then you get 'beamed up'
	+ enemies:
		- Not that much enemies. Need to add more.
		- Simple bugs that go up and down and other go left and right
		- Rocket launchers that keep shooting rockets at a certain interval and the rockets follow the enemy the change of angle is small. So the rocket turns slowly.
		- Lazers that hit the walls and then stop and possibly rotate
		- Rotating fans that kill you
	+ Interactions:
		- Player interacts with enemies, keys, powerups, pickups.
		- I'm gonna add coins so I can add some more things involving the coins as a pickup
		- Rockets aim at the players
		- Small npc's have 1 line of text and talk when the player enters the area.
		- Arrows that when hit launch the player upwards or in the direction of the arrow.		
		- Stars that do the same thing but with a different visual.
		- Teleport field that let's you trhough certain parts.
		- Sticky walls that the player can hold onto and when space is pressed slide upwards

	+ Pick-Ups:
		- Wings pickup
		- need to invent own pick ups: Magnet to collect coins faster, double points. Invincibility, remove all rockets
	+ HUD: 
		- Shows objective when in a boss battle.
		- Shows a timer
		- Maybe show score? Active powerup? Enemies killed? Points received
		

Required Applied Physics functionality:
	+ Force fields: Ventilators blowing you upwards.
	+ Triggers: Opening a locked part with that key. Key is the trigger. start firing rockets is probably also a trigger.
	+ Gliding level piece exists.
	+ Kinematic actor like the enemies that only move left and right and up and down
	+ Physics Joint:  Rotating fan is a physicsJoint. Implement elevator. Moving platforms and stuff like that.