Remember to pack the resources prior to first compile, otherwise you will get
an error about missing include/app_resources.h



Random thoughts:

Should not need to change anything at all in pulse_app.c/h. If you do, let me
know.

See watch_modes.i for controlling the watch modes that are part of your watch.

See mode_demo.c/h for a basic example exposing all the basic features. Not a
pretty watch face!

See mode_nixie.c/h or mode_powerpoint.c/h for more fully featured examples.

With just the minimum number of funcitons compiled in and no framework the
watch has around 5590 bytes of free flash space. You can see how much each
watchface consumes (worse case) in the spaceused.txt file. Faces that use
the same pulse_* functions as other watches will use less combined, but this
should give you a starting point for working out how much you can get onto
your watch.


In your watch mode code:

You may want to start with a copy of the mode_demo example and customise it to
your needs. It is worth deleting all the unused functions from the watch
function table - for example if you do not need to handle the COLDBOOT or
BLUETOOTHREC functions then just delete them and that will save valuable
watch storage space.

Use the macro multi_debug rather than dbg_print. dbg_print uses bytes in the
final image whereas multi_debug does not.

Never use pulse_register_timer or pulse_cancel_timer. Instead use
multi_register_timer and multi_cancel_timer. The multi functions automatically
cancel your timers when the watch goes into sleep or the watch mode changes,
and will never trigger if a particular watch mode is not the active one.

The only time you would use pulse_register_timer/cancel_timer would be in a 
notifications mode where the watch mode isn't real and does not have a normal
display face. 

Each time your watch mode wakes from sleep you will need to re-register
all your timers (multi...); in your MODEINIT function you need to set all the
timer id variables to -1, and in your BUTTONWAKE you need to register them.
Cancelling a -1 timer is fine, but if you forget to clear them then best case
you'll catch an assert in the simulator, worse case (should be) your cancel is
ignored 99/100 times, the other 1/100 someone elses timer will be cancelled...

There is already a looping function defined. It's set to 200ms currently,
though you can adjust it (see pulse_app.h for the exposed variable
multiLoopTimeMS). mode_demo overrides and sets to 1000ms as it is not showing
the time.
200ms was chosen so you can be reasonably accurate if you display seconds on
your watch face (~199ms, likely to be roughly the same for the time period your
watch is powered on so the end user should not notice).
If you want more accuracy then reduce this number and/or implement your own
looping function and set multiLoopTimeMS to 0 in your MODEINIT function.

Never call pulse_register_callback as that is already done in pulse_app.c.
Again, an execption might be needed to handle pulse protocol data.

Lastly, if you have more than a couple of watch faces then you will find you
do not have enough space if you want to use the pulse_render_text function.
That function consumes approximately 1554 bytes.
