Goal: Gain hands-on experience with C++ threads in a very simple graphics environment.

Task: Our homework is inspired by the former Facebook game called Farmville. If you were a developer working on that project, you might create a thread to animate a chicken: it would peck around in the yard, maybe find a nest and lay an egg in it, etc. Each action updates the position of the chicken image in the game state (a graph of objects and their current image), and then the game redraws the whole game state at some frequency, perhaps 30 frames per second or 60 frames per second.

Given code: We are providing a framework that utilizes the CUGL graphics package from CS 5152. In FarmLogic.cpp, it includes a program intended to illustrate how you’ll use it – but the scene it shows makes no real sense! It really just illustrates the kinds of predrawn objects available to you, and sort of scatters them around, and animates a few of them in minor ways. For example, this is a screenshot we made of it:

Some graphics rules: This project is using CUGL from CS 5152 (link). Most of the graphics have been abstracted away, so students who wish only to complete the threading exercise do not need to worry about it. However, there will be an extra credit incentive for those who wish to make their farm simulation look nice (detailed below). 


All objects are represented as rectangles with a width and a height
The positions of objects are center anchored. For example, an chicken at (x,y) = (100,100) with (w,l) = (40,40) would have verticies at (80,80),(120,80),(120,120),(80,120)
The Farm is 800 pixels wide and 600 pixels tall. The bottom left is (0,0), and the top right is (800,600)


Each object has an ID field. This value should be unique for all objects
Each object has a layer field. Layer 0 is for stationary background objects (like nests, ), and layer 1 is for moving objects
Objects in the same layer should not collide. We will be testing for this
Each object has a texture. Upon a status update for that object, the texture should change. For example, if a chicken lays an egg in a nest, the nest's texture should change to an image with an egg laid in it



The scenario:
• We have a set of farms that produce eggs, flour, butter and sugar. The screen definitely has room for two farms. So we will have one that produces butter and eggs, and a second farm that produces flour and sugar. On a farm there will be a few “prebuilt” nests, initially empty. You definitely will want at least two nests but more may be useful.
• Eggs are sold by the crate, consisting of 3 eggs. The farm can produce “unlimited” amounts of butter, and the flour/sugar farm makes lots of flour and sugar, but eggs need to match what the chickens lay.
• You should have a non-trivial amount of animals visible: at least four chickens and
three cows. Chickens basically walk from nest to nest, wait for space, and then lay one or more eggs
in the nest. A nest with 3 eggs is full. You will want a pretty rapid supply of eggs. Even so, the
chickens should sometimes move around – they can’t just sit on a nest laying eggs continuously.
At a minimum, your chickens must change to a different nest at least once every 3 eggs laid, and
they do this by walking, not teleportation.
• From time to time the farmer comes and collects the eggs. This empties the nests.
• You can animate the cow, or just have her standing around. The farmer actually could milk the
cow now and then, but you would need to add animations in CUGL for that. We are not requiring that you show the farmer milking the cow, but the best looking simulated farms will receive extra credit (detailed below)
• There should be two trucks. One of them drives back and forth to the butter and egg farm, and the
other drives to the flour and sugar farm. On arrival, each loads up with a full load of produce (see
below), then takes it to Anne’s Patisserie, where there are four conveyor belts, one for each kind
of produce. The truck unloads onto belts and when it is fully unloaded, can go fetch more
produce. A truck never needs to “wait” for butter, flour or sugar… but may need to wait for
eggs, if it arrives when the farm doesn’t have three or more crates available.
• A full load of produce “fills a truck”. This will be three of each: three crates of eggs (a total of 9
eggs), three boxes of butter, three bags of flour, three bags of sugar. Notice that the eggs are still
a limiting factor because in our setup, we can see them being laid. If the farm has extra crates
of eggs, the truck can’t take them all in one load: it would carry three, then come back for three
more, etc.
• Again, keep in mind that moving is a step by step process. A truck doesn’t teleport from the
factory to the farm: it has to follow some form of path (road) from factory to farm and back. If
you like, you can add a background image for the roads you will use. Be careful that your trucks aren’t too slow: if they are, we
won’t produce a reasonable rate of cakes.
• Now, let’s focus on the factory. The four conveyor belts have a fixed capacity of 2 units (2 boxes
of butter, or 2 crates of eggs, etc). and when they are full, the truck must wait for space to unload.
They transport the product from left to right.
• There is a batter mixer. To make one double batch of batter it takes two crates of eggs, two sacks
of flour, two boxes of butter and two bags of sugar. Then the batter is automatically mixed. This
double batch will be baked in two parts, making 3 cakes each time. After that, the mixer is free
and ready for more ingredients.
• There is an oven. You can bake a batch of cakes only if the oven is free (no prior batch is baking)
and also if there is room for the batch in the “front section” of the pastry shop. This front area
has a tray with room for six cupcakes (well, maybe full size cakes!) and each baked batch is three
cakes.
• Children come to buy the cupcakes. They can buy as few as one cake or as many as six, randomly.
Only one child can enter the pastry store at a time. There should be five
children, and all of them stand around waiting to buy a cake, then walk away (to take to be eaten),
and then return for another cake. We should be able to see all five children at all times. A child
will wait (in the store) if he or she is trying to buy k cakes, but there are currently less than k in
the tray. When more are baked, that child continues to buy them until it reaches the target
number. So for example, a child who wants 5 cakes and enters when there is just 1 left would
need to wait for two more batches (3 cakes each!) to be ready. Then there would be 2 remaining
when that child leaves the store (1 + 6 – 5). As the number of cakes changes, redraw the tray with
the proper number in it.
• In addition to displaying the farm, we also track how many eggs have been laid and how many
have been used up (the remainder would need to be in a truck or on the conveyor belt), how
much butter, etc. The statistics are shown at the bottom of the screen. The system just runs
endlessly, but shouldn’t ever “lose” products!


Notice the various synchronization conditions!
• We aren’t allowing children or chickens or trucks or other objects at the same layer to occupy the
same space at the same time. You’ll need to enforce this. If your trucks follow roads that cross,
they will need to be careful at the intersections or a crash could occur!
• A nest isn’t allowed to overflow: once it has 3 eggs in it, that nest is full until the farmer empties
it. If a nest ever has 4 eggs, that would be a synchronization error.
• A truck must be emptied before it can do its next trip to the farm, but emptying it requires space
on the conveyor belt for all the products it carried. It must be full when it leaves the farm.
• The mixer can’t mix until it has the required ingredients. Then the batter has to be baked before
it can be emptied and start a new batch.
• The oven can’t be used until there is room for another batch of baked goods in the front store.
• There may be additional requirements that we haven’t mentioned, for example to avoid having
chickens or trucks crash into each other. (And you are welcome to extend the basic setup, but if
you do, it would probably add more synchronization requirements).
• There are also C++ language synchronization requirements. Look for variables that need to be
updated or read in critical sections and be sure to protect them properly!


Your job:
• Our existing program has a displayable object class and creates some basic objects, which it
displays in a pretty random way so that you can see them. Then it loops animating one or two
things, again in a totally random way to demonstrate the capability. Some of the objects evolve:
nests can have varying numbers of eggs, and in fact there is an “object” for the mixer contents
that holds a string showing what is currently in the mixer (flour is shown as an F, butter as a B,
etc). In fact a double batch of batter really needs two of each – we recommend small b for “one
butter, needs one more”, capital B if there are two butters, etc. Then the double batch has to be
cooked in two steps – a half making three cakes (which would cook and then need to be moved
to the rack in the front office), etc.
• So… your task is create one thread per animated object, which would loop and show the object
as it moves around. Use the monitor style of synchronization, and use monitor condition variables
to wait for specific things.
• Additionally, do the redisplay action in a separate thread that loops: it should redisplay, sleep for
a while, then repeat.
• PLEASE NOTE: Our main program (as given to you) uses usleep. This is NOT the way for threads
to pause – it pauses the whole application. Your threads will be using the condition variable
wait_until operation, which lets you wait for a timed delay. You should read about wait_until,
and eliminate usleep completely from your new main process. 


Part 1: Due on X. For this part of the assignment, we want you to implement all the needed
threads to do concurrent animation of all the moving parts, with proper layout on the screen (you have
to decide where to put each thing), but without implementing any of the logic for threads interacting with
each other. For example, you won’t worry about chickens walking right over each other, or trucks
colliding. You won’t worry that the mixer needs to coordinate with the conveyor belts or even that it
needs two units of each ingredient to make a batch of batter – just have it work randomly, like in our given
code. Basically, any rule in the application that involves two threads talking to one another is in part 2,
and if you are unsure, just ask on Piazza.

Have redisplay called from a separate thread that loops, redisplays, sleeps for a while, then repeats.
Even so, there is one form of synchronization required! Our “updateFarm” method is not thread safe, and because
the underlying display image shows every object, needs to be protected so that (a) two threads never call
updateFarm on the identical object, and (b) if redisplay() is running, nobody can call updateFarm, and vice-versa. Part 1
will be buggy if you do not implement this one form of mutual exclusion.

The simulation should run until the window is closed

Part 2: Due on X: For this part, add to your part 1 all the missing
logic for all the synchronization required to fully implement the application.
What we will evaluate:
• For both parts, we will run your program and make sure that the animation seems to be correct
and implementing our various rules. For part 1 we will also look at your logic for ensuring that
redisplay() and updateFarm() do not both enter the critical section concurrently, do not deadlock, and
do not livelock. This is the only thing we will evaluate on part 1.


These other requirements will be evaluated only for part 2:
• We will check that you aren’t losing produce (like eggs that vanish).
• We will also check to see that your code has no deadlocks or livelocks caused by the extra
synchronization required to implement part 2.


requirements for part 2:
1. At least one situation where chickens must check for and coordinate with other chickens to avoid
"crashing into one-another"
2. At least one situations where two or more chickens contend for the same nest
3. At least one nest where the eggs would not come all from one chicken (e.g. not "one chicken lays
all 3 eggs" but "1 egg was from chicken A, the other 2 from chicken B").
4. You must protect against excess eggs per nest. A chicken can't lay a 4th egg in a nest.
5. The farmer cannot collect eggs from a nest while a chicken is sitting on it.
6. A truck cannot collect produce that has not actually been produced yet, particularly eggs and
butter. (You can make the rule that each time the farmer milks the cow, that results in 2 cases of
butter -- I think hw4 didn't pin that one down?).
7. The trucks must have some potential for reaching some form of intersection at the same time, and
must coordinate so that first one goes through, then the other, and it can't always be that truck B
always waits until after truck A passes. The rule has to depend on who gets there in what order
and when, not some kind of rigid thing that might leave truck B waiting ten minutes until truck A
happens to pass, for example.
8. Trucks can't leapfrog while delivering goods (but in fact you actually could reorder the conveyor
belts if you wish, as long as your road has an intersection that both trucks share, or a segment that
both share).
9. A conveyor belt can't become overly full and must wait for the mixer to have a need for its product
in order to unload something into the mixer. (So, a truck may actually have to wait for space when
unloading onto a conveyor belt).
10. The mixer can't mix without the full recipe, and it must be the proper recipe, not some random mix
of products. Two batches must bake for the mixer to be empty again.
11. The oven can only be used if the baker is prepared to remove the cakes (has room for them in the
patisserie front room)
12. A child who has his or her turn to buy cakes gets to wait until he or she has the proper number of
cakes
13. Children can't walk over one-another.



virtualbox steps