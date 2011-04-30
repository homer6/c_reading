--
-- Renames selected objects to lower case
-- Version 1
--
-- Author: Jani Kajala (jani.kajala@helsinki.fi)
--
function toLowerCase instr =
(
	local upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	local lower = "abcdefghijklmnopqrstuvwxyz"

	outstr = copy instr
	for i=1 to outstr.count do
	(
		local j = findString lower outstr[i]
		if ( j != undefined ) do outstr[i] = lower[j]
	)

	return outstr
)

utility RenameToLowerCase "Renames selected objects to lower case"
(
	button actionBtn "Do it"

	on actionBtn pressed do 
	(
		for obj in selection do 
			obj.name = toLowerCase obj.name
	)
)
