# Among Us 2D

Implemented maze game inspired from among us in 2D there will be an impostor, a green button to kill impostor, a yellow button two spawn coins and obstacles. obstacles reduce health by one unit can be seen on health bar which is at left corner of string coins increase score. The main task is to kill the impostor before it kills you by reaching the button and pressing it. Impostor chases you using shortest path algorithm (BFS).

Impostor moves after every 0.5 seconds he is fast because he only uses shortest path. The game ends in following cases

<ol>
	<li>When Player reaches the end point after solving two tasks</li>
	<li>When Impostor reaches player there by killing</li>
	<li>When player loses health because of obstacles which are created by task 2</li>
</ol>

The  game ends only when player completes both the tasks and reaches to end position with health > 0.

## CONTROLS
* **W** to move up
* **S** to move Down
* **A** to move left
* **D** to move Right
* **K** to press the button
