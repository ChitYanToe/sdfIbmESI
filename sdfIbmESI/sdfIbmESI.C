/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2017 OpenFOAM Foundation
    Copyright (C) 2019 OpenCFD Ltd.
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Application
    sdfIbmESI.C

Group
    grpIncompressibleSolvers

Description
    Transient solver for incompressible, turbulent flow of Newtonian fluids
    with immersed boundary method.

Reference: Chenguang Zhang. sdfibm: a Signed Distance Field Based Discrete Forcing Immersed Boundary Method in OpenFOAM. Computer Physics Communications (accepted 2020) 
\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "singlePhaseTransportModel.H"
#include "turbulentTransportModel.H"
#include "pimpleControl.H"
#include "fvOptions.H"
#include "localEulerDdtScheme.H"
#include "fvcSmooth.H"

#include "solidcloud.H"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addNote
    (
        "Transient solver for incompressible, turbulent flow"
        " of Newtonian fluids on a moving mesh."
    );

    #include "postProcess.H"

    #include "addCheckCaseOptions.H"
    #include "setRootCaseLists.H"
    #include "createTime.H"
    #include "createMesh.H" // chit added
    #include "initContinuityErrs.H"
    #include "createFields.H"
    #include "createUfIfPresent.H"
    #include "CourantNo.H"

    std::string	dictfile;

    turbulence->validate();

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    // if start-time > 0, read from start-time-folder for solidDict, otherwise read from case root
    if(runTime.time().value() > 0)
    {
	    if(!Foam::Pstream::parRun())
		    dictfile = mesh.time().timeName() + "/solidDict";
	    else
		    dictfile = "processor0/" + mesh.time().timeName() + "/solidDict";
    }
    else
    {
	    dictfile = "solidDict";
    }

    sdfibm::SolidCloud solidcloud(args.path() + "/" + dictfile, U, runTime.value()); // chit
    solidcloud.saveState();  // write the initial condition edited by chit
    
    Info<< "\nStarting time loop\n" << endl;

    while (runTime.loop())
    {
        Info<< "Time = " << runTime.timeName() << nl << endl;
	Foam::dimensionedScalar dt = runTime.deltaT();
        
	// --- Pressure-velocity PIMPLE corrector loop
        if(solidcloud.isOnFluid())
	{
		Foam::fvVectorMatrix UEqn(
                fvm::ddt(U)
              + 1.5*fvc::div(phi, U) - 0.5*fvc::div(phi.oldTime(), U.oldTime())
              ==0.5*fvm::laplacian(nu, U) + 0.5*fvc::laplacian(nu, U));
            UEqn.solve();

	    phi = linearInterpolate(U) & mesh.Sf();
	    Foam::fvScalarMatrix pEqn(fvm::laplacian(p) == fvc::div(phi)/dt - fvc::div(Fs));
            pEqn.solve();

	    U   = U   - dt*fvc::grad(p);
            phi = phi - dt*fvc::snGrad(p)*mesh.magSf();

            Foam::fvScalarMatrix TEqn(
                fvm::ddt(T)
              + fvm::div(phi, T)
              ==fvm::laplacian(alpha, T));
            TEqn.solve();
	}

	solidcloud.interact(runTime.value(), dt.value());

	if(solidcloud.isOnFluid())
        {
		U = U - Fs*dt;
            	phi = phi - dt*(linearInterpolate(Fs) & mesh.Sf());

         	U.correctBoundaryConditions();
            	adjustPhi(phi, U, p);

     		T = (1.0 - As)*T + Ts;
		T.correctBoundaryConditions();
		#include "continuityErrs.H"
	}

	solidcloud.evolve(runTime.value(), dt.value());
	solidcloud.saveState();
	
	if(solidcloud.isOnFluid())
	{
	    solidcloud.fixInternal(dt.value());
	}

	if(runTime.outputTime())
        {
            runTime.write();

            if(Foam::Pstream::master())
            {
                std::string file_name;
                if(Foam::Pstream::parRun())
                    file_name = "./processor0/" + runTime.timeName() + "/solidDict";
                else
                    file_name = "./" + runTime.timeName() + "/solidDict";
                solidcloud.saveRestart(file_name);
            }
        }

    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
