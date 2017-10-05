# Harmony
## *A permanent temporary solution*

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)


Harmony is a end-to-end encrypted group messaging based on the [(n+1)sec protocol][n+1 link]. It offers distrubuted communications that is both deniable and is forward-secret.

### Information
Harmony was built by a team of 5 during Husky Hacks 3, a 36-hour hackathon at Northeastern Univeristy. The team consisted of Erik Uhlmann, Edward Shen, JJ Bernhardt, Elijah Steres, and Cameron Kennedy. It utilizes the [Spread Toolkit (v4.4.0)][s-tk] for managing the network, and the (n+1)sec protocol to manage our encrypted messages.

Harmony is the code that interfaces the Spread ToolKit with [the reference implementation of (n+1)sec][n+1 implement] and packages it into a nice user interface made in Qt5.

### Installation
You need `libgcrypt` in your path and Qt5 installed.

... Good luck. You probably need to `cmake` some stuff and `make` after. We're just happy it built on our systems.

Just know if you somehow do get the binaries to compile, you need to run spread as
```bash
# spread -c test.spread-conf
```

Also for linux users:

You need to install `libspread` and `libspread-core` and make sure it's in your `LD_LIBRARY_PATH`. What one of our developers did (as a hack) was to copy the library files over to a build folder, and manually added the build folder to the `LD_LIBRARY_PATH`. We'd like to get this fixed but for a time-limited event this was the best we could do.

### Credits and Acknowledgements
The Spread Toolkit is under a modified BSD v3 license. As such, we are legally require to state the following:

This product uses software developed by Spread Concepts LLC for use in the Spread toolkit. For more information about Spread, see http://www.spread.org

(n+1)sec is under [![License: LGPL v3](https://img.shields.io/badge/License-LGPL%20v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0).

Qt is under [![License: LGPL v3](https://img.shields.io/badge/License-LGPL%20v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0).

[n+1 link]: https://learn.equalit.ie/wiki/Np1sec
[s-tk]: https://spread.org/
[n+1 implement]: https://github.com/equalitie/np1sec
