# Advanced morphological filtering algorithms for OpenCV

OpenCV does not provide generalized morphological attribute filtering. This
project implements attribute filters based on the OpenCV libraries.

## What is mathematical morphology?

Mathematical morphology is the studies of shapes or connected components on binary or gray-scale images. Read [Jean Serra and Luc Vincent: An overview of morphological filtering](http://www.citeulike.org/user/fbie/article/3377121).

The algorithms implemented here are advanced morphological filters that do not
rely on structuring elements. Instead, more general and parameterized measures
are used for filtering. The most basic filter of this kind is morphological area
opening and closing, where elements up to a certain area are filtered from the
image.

## How to build it?

You need [SCons](http://www.scons.org/) to build. Once you got that, simply run
something like

```
$ scons mode=release -j8
```

to build the library in release mode.

## How to use it?

Link against the morphology libraries and either use the convenience functions
for area opening and closing by including `morphology/Filters.h` or
`morphology/AttributeFilter.h`. The latter one will increase compilation time,
but you can write your own fancy attributes.