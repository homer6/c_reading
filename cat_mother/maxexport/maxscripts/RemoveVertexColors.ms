--
-- Removes vertex colors from selected objects
-- Version 1
--
-- Author: Jani Kajala (jani.kajala@helsinki.fi)
--
utility RemoveVertexColors "Removes vertex colors"
(
	button removeBtn "Do it"

	on removeBtn pressed do 
	(
		for obj in selection do meshop.setNumCPVVerts obj 0
	)
)
