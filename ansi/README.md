Console
=======


A `Terminal` represents an end point, a device to which you can write bytes and read bytes. It is assumed the terminal deals with "raw" input
and not line based. A read() should return directly if no characters are available.

A `Console` is a (presumably visible) 2D grid of text with attributes.

A `Protocol` takes a console command and translates it into a stream of bytes intended for a specific Terminal.

