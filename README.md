# X16 Playground

This relies on libX16 for talking to the VERA chip and doing stuff on the Commander X16.
This tests several things, like pixel drawing, line drawing, rectangle drawing, clearing screen and also has a function for copying memory to VRAM (thanks Mark-BugSlayer on Discord)

Use image2binary from https://github.com/TurboVega/image2binary to convert the image to their respective palette and RAW image.

# Why

The X16 supports a bitmapped 8bpp mode along with the following resolutions:
- 160x240
- 320x240
- 640x480

There's an issue with all of them relating to the fact that it can't hold the entire framebuffer in one bank,
and that also applies to the 160x240 mode as well.

This restriction is less noticeable if you use VLOAD as it does take care of that but if you are
pixel drawing on the screen, that's something to keep in mind.

This restricts you to 160x232 or 320x208 or so without handling this properly.
I decided not to handle this because for 640x480, it's likely that you just want to show a static screen or use the tiled modes instead
and the CPU is already slow as it is.

Eventually i would like to port Anarch to it and start at first with pixel dra
