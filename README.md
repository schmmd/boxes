# Boxes

A faithful web port of **Boxes**, the 2002 Windows game by Michael Schmitz.

Cage the squirrels by pushing lines of boxes around them with the arrow keys.
But beware: the squirrels bite. As the levels increase not only do the squirrels
multiply, but they quicken and the boxes needed to confine them become scarce!

The squirrels move on a timer, so the game is real-time — a squirrel is caged
once every one of its eight neighbours is a box, a wall, or another squirrel.
When every squirrel on the board is caged you advance to the next level.

## Play

- **Arrow keys** — move / push boxes (or **swipe** on touch screens)
- **Enter** — continue after winning / retry after dying
- **Ctrl+N** — New Game &nbsp; **Ctrl+A** — New Arcade Game &nbsp; **Ctrl+P** — Pause
- **?** — Instructions

The original sprites (the surprised smiley, the crate, the squirrel and all seven
win faces) were extracted directly from `boxes.exe` for authenticity. Your best
level and time are saved locally in the browser.

## Deploying via GitHub Actions

This branch (`web`) contains only the files needed to serve the game. Pushing to
`web` triggers `.github/workflows/deploy.yml`, which publishes the site to
GitHub Pages.

One-time setup: in the repository, go to **Settings → Pages → Build and
deployment** and set **Source** to **GitHub Actions**. After the next push to
`web`, the game is served at the URL shown in the workflow's deploy step.

## Files

```
index.html            markup, menu bar and dialogs
style.css             classic Win9x styling
game.js               game logic (ported from the original C++)
sprites/*.png         original sprites extracted from boxes.exe
.github/workflows/    GitHub Pages deployment
```
