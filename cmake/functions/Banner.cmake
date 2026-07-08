## libarcsdec: let cmake print a colored banner
## vim:fdm=marker
## inspired by https://stackoverflow.com/a/19578320

cmake_minimum_required (VERSION 3.10 )

if (NOT WIN32 )

	string (ASCII 27 Esc )

	set (ColourReset "${Esc}[m"     )
	set (ColourBold  "${Esc}[1m"    )
	set (Red         "${Esc}[31m"   )
	set (Green       "${Esc}[32m"   )
	set (Yellow      "${Esc}[33m"   )
	set (Blue        "${Esc}[34m"   )
	set (Magenta     "${Esc}[35m"   )
	set (Cyan        "${Esc}[36m"   )
	set (White       "${Esc}[37m"   )
	set (BoldRed     "${Esc}[1;31m" )
	set (BoldGreen   "${Esc}[1;32m" )
	set (BoldYellow  "${Esc}[1;33m" )
	set (BoldBlue    "${Esc}[1;34m" )
	set (BoldMagenta "${Esc}[1;35m" )
	set (BoldCyan    "${Esc}[1;36m" )
	set (BoldWhite   "${Esc}[1;37m" )

	## BEGIN, END
	function (banner )

		list (GET ARGV 0 BannerType )

		if (BannerType STREQUAL BEGIN ) #OR BannerType STREQUAL END )
			list (REMOVE_AT ARGV 0 )
			message ("${BoldBlue}${ARGV}${ColourReset}" )
		elseif (BannerType STREQUAL END )
			list (REMOVE_AT ARGV 0 )
			message ("${BoldGreen}${ARGV}${ColourReset}" )
		else ()
			message ("${ARGV}" )
		endif ()
	endfunction ()
else ()

	function (banner )

		list (GET ARGV 0 BannerType )
		list (REMOVE_AT ARGV 0 )

		message ("** ${BannerType}: ${ARGV}" )
	endfunction ()
endif ()

