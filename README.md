# Xmas-Box
Toy project implementing a simple xmas calendar based on RFID.
## Outline
The idea is to present a new RFID card each day and receive some entertainment in return. Entertainment meaning listening to sounds like (randomly selected) music or a radio play. 24 cards associated with 24 pieces are assumed.

To provide entertainment (see definition above) for multiple persons, another set of RFID card may be used to select a certain play.

A special treat is reserved for Xmas day as one RFID card starts a final sound track, i.e. a typical xmas story.

Finally, one may present a gift tag with an RFID chip. A sound file related to this tag is played. 

## States
For folder an file references see also below...

The device starts up by showing some text followed by a startup screen accompanied by some sound (Folder 99/File 001).

A text demands an RFID card to be presented, followed by a graphic that should express the same intention. This graphic will be displayed while the main loop waits for RFID input.

RFID tags either:
- switch mode
- indicate a day
- start the Xmas special
- indicate a gift tag

### Mode
The default mode plays some random music (Folder 80). Other modes are associated with some tracks (i.e. a radio play or music collection). Each track consists of 24 pieces mapped on the days.

## Blueprint
Hardware and wiring are pictured [here](./X-Mas-Calendard.fzz).

The code consists of one arduino ino file and some header files. Header files contain bitmaps and some texts.

## Adapt code
As long as one uses the hard wired folder structure for the mp3 player, only the number of provided tracks are to adapt.

Graphics are stored in the header files. The code references them in the function `displayPic`. Define statements need to be adjusted. 

Separation between chars.h and pictures.h is arbitrary. The later is used for full screen pictures and animations, the former for smaller icon style pictures needed to animate the screen while the mp3 player does his work.

### Folder structure
Sound files are organized in folders. The mp3 references only the folder numer (2 digits) and the file number (3 digits).

The following folders and files are needed in the standard setup:
 - Folder 01 - 24: Daily track files (001 - 099) and countdown (100).
 - Folder 71-79: Some announcements to be played when a gift tag is presented. Number of announcements in these folders must be equal. Number is reflected in code ``#define numTagReaderTracks ...``
 - Folder 80: Music files (001 - 999). Number of music files must be reflected in code ``#define numMusicTracks ...``
- Folder 90: Sound file to be played on Xmas (001)
- Folder 98: Jingles to be played when the mode is changed (001 - )
- Folder 99: 001 = Startup sound

### RFID tags
The tags need only one byte to be set. In the code it is sector 1, block 4, 1st byte (that's as arbitrary as it can get :-) ).
- 1-24 denotes days
- 71-79: codes for gift tags to be read
- 99 is a special code for xmas day
- 100 is the random music mode
- 101 - 103: tracks (mode)

 