If you wish to use another version of the engine other than ENGINE.LIB, you
will have to make some simple modifications.

at loadpalette(), replace the PALETTE.DAT loading code with these calls:

loadpalettes();
loadlookup();
loadtrans();

at qloadkvx(), replace these functions:

kopen4load() ---> loaditem();
kread() ---> readitem();
kfilelength() ---> itemlength();
kclose() ---> closeitem(); 

Remove this at line 6989 (or at makepalookup):

if (paletteloaded == 0) return;

If you want to use -sn1dev and -seqdev, you will also have to modify the
art file handling code to support directories. I don't remember how I did
it.
