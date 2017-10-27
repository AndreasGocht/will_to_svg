# will_to_svg

This is a small tool, which convertes Wacom .will files from the Bamboo Spark to .svg files.

Thanks tho wacom, who made the essential part of the .will format public available (found it by google :-) ):

https://developer-docs.wacom.com/display/DevDoc/WILL+SDK+for+ink+-+File+Format

## build

### dependencies
You'll need

* libzip
* protobuf

### compile

```
mkdir build
cd build
cmake ../
make
make install
```

This will install will_to_svg to the default location. To change the location specify `-DCMAKE_INSTALL_PREFIX`

## usage

```
will_to_svg -i input_filename [-o output_filename]
```

* `-i` input filename of the .will file
* `-o` output filename. If blank the outputname will be the inputfilename with .svg appended.  


