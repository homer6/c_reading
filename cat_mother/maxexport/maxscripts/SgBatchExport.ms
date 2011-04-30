--
-- Exports *.max -> *.sg
-- Version 1.1
--
-- Author: Jani Kajala (jani.kajala@helsinki.fi)
--
-- Options:
--
-- Files:
--     Sources file name filter, can contain wildcards.
--
-- Output:
--     Receives exported files, name must end with \ if directory.
--
-- Remove version suffix:
--     Removes _1, _01 and _001 style version numbers at the end of exported filename
--
--
-- Changes:
-- 1.1: 
-- - Better version suffix removal
--

function isnum ch =
(
	local numbers = "0123456789"
	return (findString numbers ch) != undefined
)

utility SgBatchExport "sg Batch export"
(
	edittext sourcePath "Files" text:"\\batch_export_temp\\*.max"
	edittext outputPath "Output" text:"\\batch_export_temp\\out\\"
	checkbox removeEndSuffix "Remove version suffix" checked:true
	button removeBtn "Batch Export"

	on removeBtn pressed do 
	(
		maxfiles = getFiles sourcePath.text
		for maxfile in maxfiles do 
		(
			ok = loadMaxFile maxfile useFileUnits:true
			if ( ok ) do
			(
				srcname = getFilenameFile maxfile;
				if ( removeEndSuffix.checked ) do
				(
					local suffix = srcname.count
					while ( suffix > 0 and isnum(srcname[suffix]) ) do
					(
						suffix = suffix - 1
					)
					if ( suffix != srcname.count ) do
					(
						suffix = suffix - 1
						if ( suffix > 0 ) do
						(
							srcname = substring srcname 1 suffix
						)
					)
				)

				fname = outputPath.text + srcname + ".sg"
				exportFile fname #noPrompt
			)
		)
	)
)
