#include "fvCFD.H"
#include "solidcloud.H"

int main(int argc, char *argv[])
{
	argList::noParallel();
    	argList::addOption("name", "word", "Name of field to be initialized");
	
	#include "setRootCase.H"
    	#include "createTime.H"
    	#include "createMesh.H"

	if (!args.checkRootCase())
        Foam::FatalError.exit();

	Foam::word field_name;
	if(args.options().empty())
	{
	    field_name = "alpha.water";
	    Foam::Info << "* No field name provided, default to " << field_name << "\n";
	}
	else
	{
	    args.readIfPresent("name", field_name); //chit changed new method by removing old method
	    Foam::Info << field_name << "\n";
	}
	
	sdfibm::SolidCloud solidcloud("solidDict", mesh);
	solidcloud.writeVOF(field_name);
}
