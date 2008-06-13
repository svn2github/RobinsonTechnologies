** To build Novashell **

** WINDOWS **

Install or checkout Clanlib 0.8.1 or newer from www.clanlib.org.  Also download the related precompiled
binaries you will need from here:  http://clanlib.org/download-binaries-win32-vc80.html

Make sure the examples work ok and everything compiles.

Check out the Novashell SVN tree somewhere, load clanlibstuff/novashell/source/novashell.sln with VC8.

You may get errors about missing projects like "clanCore" or "clanDisplay", you can just ignore and remove these, 
sometimes I have those projects added for when I debug clanLib and Novashell together.

You may need to add the path to Clanlib's libs (or put them in SharedLib/lib and SharedLib/include where it expects
them), but otherwise it should compile ok, as everything else uses relative paths and is included.
(LinearParticle, Lua, etc)

You should be able to click build from inside VS 2005 and have everything build, then copy the .exe over
to a novashell install to try it.

Note:  Build Static MT Release or Static MT Debug.  Ignore the other ones...

--- TO GET WINDOWS BUILD SCRIPTS/ETC WORKING (not required!)

The .bat file novashell/scripts/MakeReleaseBetaInstaller.bat is a script that will build the game,
 create the docs, create an installer and FTP it to my site.  
 
 To use this, you would need to do the following:
 
  * Add <checkoutdir>util to your system path
  * Install NSIS 2.35+ in <checkoutdir>util/NSIS
  * Install the "AccessControl" NSIS plugin (the installer sets some permissions to make it easier
    for users to edit files)
  * install naturaldocs in <checkoutdir>util/naturaldocs  (http://naturaldocs.org/ )
  
Create a batch file in <checkoutdir> called SetFTPLogonInfo.bat with the following:

SET _FTP_SITE_= rtsoft.com
SET _FTP_USER_= username
SET _FTP_PASS_= password


** OSX **

Checkout Clanlib 0.8.1 from SVN from www.clanlib.org.  Also download the related precompiled
binaries you will need from here: http://clanlib.org/download-binaries-osx-gcc40-universal.html

After checking out the Novashell SVN tree, open clanlibstuff/novashell/mac/novashell.xcodeproj, you may need to
remove the Clanlib packages and re-add them from wherever you installed and built clanlib.

-- Updating from SVN/building/packing from the command line (Not required!) 
The script scripts/mac_build_and_package_release.sh will SVN update everything, build Clanlib, build Novashell, package
it up into a dmg and upload it.  You'd have to edit the scripts involved to get this working...

** LINUX **

Checkout Clanlib 0.8.1 from SVN from www.clanlib.org.  You'll also need to grab the additional libs clanlib needs, but
hey, you're a linux guy, you can probably figure out the dependencies.

Next, check out *JUST* the novashell (and sub dirs), then, from inside that directory, check out "SharedLib".

So, instead of what the SVN has, ie:

<root checkout>
		clanlibstuff
			novashell
				Misc Novashell dirs
		SharedLib

you have:

novashell
	Misc Novashell dirs
	SharedLib
	
The reason was I couldn't get KDevelop to let me include sources that weren't part of the projects subtree.. dumb!

Anyway, next use KDevelop to open novashell.kdevelop and you're all set with the IDE and should be able to compile it.

-- Updating from SVN/building/packing from the command line (Not required!) 
script/linux_build_all_and_ftp.sh does the work.  Would need editing etc.



Good luck!

-Seth