fontconvert
============

Prerequisite you need to install python3 and install `freetype-py` using pip
The approximate process is like this:

.. code-block:: shell

    sudo apt install python3-pip
    python3 -m pip install freetype-py

The previous is the prerequisite for implementation, and then you need to store the font file you want to convert in the same path as fontconvert.py. This is just for more convenient operation. You can also fill in the font path.

Then just follow the command below to convert the font.

.. code-block:: shell

    python3 fontconvert.py --compress demo 16 msyh.ttc > font.h

Explanation of specific parameters:

.. code-block:: shell

    python3 fontconvert.py --compress [generated font name] [font size] [font file path] > [generated font file]

Of course, this only demonstrates the generation of standard ascii codes. If you need other fonts, you only need to fill in the unicode encoding of the font to be generated in the `fontconvert.py`  `intervals` list.

Please make sure that the unicode encoding in the fontconvert.py intervals list is included in your font file, otherwise please comment other encodings and only keep the 32,126 range!

.. image:: https://user-images.githubusercontent.com/22990954/104092951-446c1880-52c2-11eb-852e-8c50626eb340.png

imgconvert
============

1. Prerequisites

.. code-block:: shell

    python3 -m pip install pillow

2. Instructions

.. code-block:: shell

    imgconvert.py [-h] [-i INPUTFILE] [-n NAME] [-o OUTPUTFILE]

3. examples:

.. code-block:: shell

    python imgconvert.py -i demo.png  -n demo -o demo.h
