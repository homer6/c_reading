/**
 * Compares nodes by name.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */ 
class NodeNameLess
{
public:
	bool operator()( INode* a, INode* b )
	{
		return strcmp( a->GetName(), b->GetName() ) < 0;
	}
};
