//
//  LEOValue.cpp
//  Stacksmith
//
//  Created by Uli Kusterer on 18/12/2016.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include "LEOValue.hpp"




void	CppVariantBase::SetAsInt( int n )
{
	this->~CppVariantBase();
	new (this) CppVariantInt(n);
}


void	CppVariantBase::SetAsDouble( double n )
{
	this->~CppVariantBase();
	new (this) CppVariantDouble(n);
}


char varnames[26] = {	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
						'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
						'u', 'v', 'w', 'x', 'y', 'z' };


void	MakeMacros( size_t maxNumArgs )
{
	stringstream	ss;
	size_t			currNumArgs = 3;
	
	ss << "#define CPPVMAX2(a,b) (((a) > (b)) ? (a) : (b))" << endl;
	
	for( size_t x = 2; x < maxNumArgs; x++ )
	{
		ss << "#define CPPVMAX" << (x+1) << "(";
		size_t y = 0;
		for( ; y < currNumArgs; y++ )
		{
			if( y == 0 ) ss << varnames[y];
			else ss << "," << varnames[y];
		}
		ss << ")\tCPPVMAX2(CPPVMAX" << x << "(";
		y = 0;
		for( ; y < (currNumArgs -1); y++ )
		{
			if( y == 0 ) ss << "(" << varnames[y] << ")";
			else ss << ", (" << varnames[y] << ")";
		}
		ss << "),(" << varnames[y] << "))" << endl;
		
		currNumArgs++;
	}
	
	cout << ss.str().c_str() << endl;
}
