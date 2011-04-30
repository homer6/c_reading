#include "StdAfx.h"
#include "MySceneExport.h"
#include "GmUtil.h"
#include "TmUtil.h"
#include "BipedUtil.h"
#include "GmModel.h"
#include "MorphUtil.h"
#include "AnimExportUtil.h"
#include "GmMaterial.h"
#include "ChunkUtil.h"
#include "SgUtil.h"
#include "LODUtil.h"
#include "SceneExportUtil.h"
#include "PhyExportUtil.h"
#include "ShadowVolumeBuilder.h"
#include "KeyFrame.h"
#include "WinUtil.h"
#include "ProgressDialog.h"
#include "AbortExport.h"
#include "Version.h"
#include "resampling.h"
#include "resource.h"
#include "NodeNameLess.h"
#include "SgLightDistanceLess.h"
#include <io/FileOutputStream.h>
#include <io/ChunkOutputStream.h>
#include <mb/Polygon.h>
#include <mb/Vertex.h>
#include <mb/VertexMapFormat.h>
#include <mb/DiscontinuousVertexMap.h>
#include <dev/Profile.h>
#include <util/Vector.h>
#include <util/Hashtable.h>
#include <lang/Debug.h>
#include <lang/String.h>
#include <lang/Exception.h>
#include <math/Vector3.h>
#include <math/Quaternion.h>
#include <math/OBBoxBuilder.h>
#include "build.h"
#include <algorithm>
#include <stdio.h>
#include <direct.h>
#include <dummy.h>
#include <plugapi.h>
#include "wm3.h"
#include "resampling.h"
#include <process.h>
#include <errno.h>

//-----------------------------------------------------------------------------

using namespace io;
using namespace dev;
using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

static int	s_dialogReturn;

//-----------------------------------------------------------------------------

/** 
 * Reads boolean value from config file.
 * Does nothing if the file handle is 0.
 */
static void readBoolean( FILE* fh, bool* v )
{
	if ( fh )
	{
		int x;
		if ( 1 == fscanf( fh, "%i", &x ) )
			*v = (0 != x);
	}
}

/** 
 * Reads float value from config file.
 * Does nothing if the file handle is 0.
 */
static void readFloat( FILE* fh, float* v )
{
	if ( fh )
	{
		float x;
		if ( 1 == fscanf( fh, "%f", &x ) )
			*v = x;
	}
}

/** 
 * Writes boolean value to config file.
 * Does nothing if the file handle is 0.
 */
static void writeBoolean( FILE* fh, bool v )
{
	if ( fh )
	{
		fprintf( fh, "%i ", (v?1:0) );
	}
}

/** 
 * Writes float value to config file.
 * Does nothing if the file handle is 0.
 */
static void writeFloat( FILE* fh, float v )
{
	if ( fh )
	{
		fprintf( fh, "%f ", v );
	}
}

static void setDlgItemTextFloat( HWND hwnd, int item, float value )
{
	char str[256];
	sprintf( str, "%g", value );
	SetWindowText( GetDlgItem(hwnd,item), str );
}

static bool getDlgItemTextFloat( HWND hwnd, int item, float* value )
{
	char str[256];
	if ( !GetWindowText( GetDlgItem(hwnd,item), str, sizeof(str) ) )
		return false;

	return 1 == sscanf(str,"%g",value);
}

static BOOL CALLBACK exportDlgProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	MySceneExport* exp = (MySceneExport*)GetWindowLongPtr( hwnd, GWLP_USERDATA );

	switch ( msg ) 
	{
	case WM_INITDIALOG:{
		exp = (MySceneExport*)lp;
		SetWindowLongPtr( hwnd, GWLP_USERDATA, lp );
		CenterWindow( hwnd, GetParent(hwnd) );

		// set dialog title
		char title[100];
		sprintf( title, "sgexport Build %i", BUILD_NUMBER );
		SetWindowText( hwnd, title );

		// load options
		char cfg[1000];
		String cfgname = exp->name() + ".opt";
		cfgname.getBytes( cfg, sizeof(cfg), "ASCII-7" );
		FILE* fh = fopen( cfg, "rt" );
		bool geometryExport = exp->geometryExport();
		bool sceneExport = exp->sceneExport();
		bool resampleAnimations = exp->resampleAnimations();
		bool textureCopy = exp->textureCopy();
		bool shadowOptimize = exp->shadowOptimize();
		float maxShadowSampleDistance = exp->maxShadowSampleDistance();
		bool forceDynamicShadows = exp->forceDynamicShadows();
		bool collapsePatches = exp->collapsePatches();
		bool exportMorphTargets = exp->exportMorphTargets();
		bool compressLightMaps = exp->compressLightMaps();
		bool staticLightingOnly = exp->staticLightingOnly();
		bool rescaleWorldUnits = exp->rescaleWorldUnits();
		float maxPositionResamplingError = AnimExportUtil::MAX_POSITION_RESAMPLING_ERROR;
		float maxRotationResamplingError = AnimExportUtil::MAX_ROTATION_RESAMPLING_ERROR;
		float maxScaleResamplingError = AnimExportUtil::MAX_SCALE_RESAMPLING_ERROR;
		readBoolean( fh, &geometryExport );
		readBoolean( fh, &sceneExport );
		readBoolean( fh, &textureCopy );
		readBoolean( fh, &shadowOptimize );
		readFloat( fh, &maxShadowSampleDistance );
		readBoolean( fh, &forceDynamicShadows );
		readBoolean( fh, &collapsePatches );
		readBoolean( fh, &exportMorphTargets );
		readBoolean( fh, &compressLightMaps );
		readBoolean( fh, &staticLightingOnly );
		readBoolean( fh, &rescaleWorldUnits );
		readBoolean( fh, &resampleAnimations );
		readFloat( fh, &maxPositionResamplingError );
		readFloat( fh, &maxRotationResamplingError );
		readFloat( fh, &maxScaleResamplingError );
		exp->setGeometryExport( geometryExport );
		exp->setSceneExport( sceneExport );
		exp->setResampleAnimations( resampleAnimations );
		exp->setTextureCopy( textureCopy );
		exp->setShadowOptimize( shadowOptimize );
		exp->setMaxShadowSampleDistance( maxShadowSampleDistance );
		exp->setForceDynamicShadows( forceDynamicShadows );
		exp->setCollapsePatches( collapsePatches );
		exp->setExportMorphTargets( exportMorphTargets );
		exp->setCompressLightMaps( compressLightMaps );
		exp->setStaticLightingOnly( staticLightingOnly );
		exp->setRescaleWorldUnits( rescaleWorldUnits );
		AnimExportUtil::MAX_POSITION_RESAMPLING_ERROR = maxPositionResamplingError;
		AnimExportUtil::MAX_ROTATION_RESAMPLING_ERROR = maxRotationResamplingError;
		AnimExportUtil::MAX_SCALE_RESAMPLING_ERROR = maxScaleResamplingError;
		if ( fh )
			fclose( fh );

		// set dialog state
		CheckDlgButton( hwnd, IDC_EXPORTGEOMETRY, exp->geometryExport() ); 
		CheckDlgButton( hwnd, IDC_EXPORTSCENE, exp->sceneExport() ); 
		CheckDlgButton( hwnd, IDC_RESAMPLEANIMATIONS, exp->resampleAnimations() );
		CheckDlgButton( hwnd, IDC_COPYTEXTURES, exp->textureCopy() ); 
		CheckDlgButton( hwnd, IDC_OPTIMIZESHADOWS, exp->shadowOptimize() );
		CheckDlgButton( hwnd, IDC_FORCEDYNAMICSHADOWS, exp->forceDynamicShadows() );
		CheckDlgButton( hwnd, IDC_COLLAPSEPATCHES, exp->collapsePatches() );
		CheckDlgButton( hwnd, IDC_EXPORTMORPHTARGETS, exp->exportMorphTargets() );
		CheckDlgButton( hwnd, IDC_COMPRESSLIGHTMAPS, exp->compressLightMaps() );
		CheckDlgButton( hwnd, IDC_STATICLIGHTINGONLY, exp->staticLightingOnly() );
		CheckDlgButton( hwnd, IDC_RESCALEWORLDUNITS, exp->rescaleWorldUnits() );
		setDlgItemTextFloat( hwnd, IDC_MAXSHADOWSAMPLEDISTANCE, exp->maxShadowSampleDistance() );
		setDlgItemTextFloat( hwnd, IDC_MAXPOSITIONRESAMPLINGERROR,	AnimExportUtil::MAX_POSITION_RESAMPLING_ERROR );
		setDlgItemTextFloat( hwnd, IDC_MAXROTATIONRESAMPLINGERROR,	AnimExportUtil::MAX_ROTATION_RESAMPLING_ERROR );
		setDlgItemTextFloat( hwnd, IDC_MAXSCALERESAMPLINGERROR,		AnimExportUtil::MAX_SCALE_RESAMPLING_ERROR );
		return FALSE;
		break;}

	case WM_COMMAND:{
		switch ( LOWORD(wp) ) 
		{
		case IDOK:{
			// get dialog state
			exp->setGeometryExport( 0 != IsDlgButtonChecked(hwnd,IDC_EXPORTGEOMETRY) );
			exp->setSceneExport( 0 != IsDlgButtonChecked(hwnd,IDC_EXPORTSCENE) );
			exp->setResampleAnimations( 0 != IsDlgButtonChecked(hwnd,IDC_RESAMPLEANIMATIONS) );
			exp->setTextureCopy( 0 != IsDlgButtonChecked(hwnd,IDC_COPYTEXTURES) );
			exp->setShadowOptimize( 0 != IsDlgButtonChecked(hwnd,IDC_OPTIMIZESHADOWS) );
			exp->setForceDynamicShadows( 0 != IsDlgButtonChecked(hwnd,IDC_FORCEDYNAMICSHADOWS) );
			exp->setCollapsePatches( 0 != IsDlgButtonChecked(hwnd,IDC_COLLAPSEPATCHES) );
			exp->setExportMorphTargets( 0 != IsDlgButtonChecked(hwnd,IDC_EXPORTMORPHTARGETS) );
			exp->setCompressLightMaps( 0 != IsDlgButtonChecked(hwnd,IDC_COMPRESSLIGHTMAPS) );
			exp->setStaticLightingOnly( 0 != IsDlgButtonChecked(hwnd,IDC_STATICLIGHTINGONLY) );
			exp->setRescaleWorldUnits( 0 != IsDlgButtonChecked(hwnd,IDC_RESCALEWORLDUNITS) );

			float fv;
			if ( getDlgItemTextFloat(hwnd, IDC_MAXPOSITIONRESAMPLINGERROR, &fv) )
			{
				AnimExportUtil::MAX_POSITION_RESAMPLING_ERROR = fv;
			}
			else
			{
				MessageBox( hwnd, "Max position resampling error: Invalid value", "sgexport - Error", MB_OK );
				break;
			}
			
			if ( getDlgItemTextFloat(hwnd, IDC_MAXROTATIONRESAMPLINGERROR, &fv) )
			{
				AnimExportUtil::MAX_ROTATION_RESAMPLING_ERROR = fv;
			}
			else
			{
				MessageBox( hwnd, "Max rotation resampling error: Invalid value", "sgexport - Error", MB_OK );
				break;
			}
			
			if ( getDlgItemTextFloat(hwnd, IDC_MAXSCALERESAMPLINGERROR, &fv) )
			{
				AnimExportUtil::MAX_SCALE_RESAMPLING_ERROR = fv;
			}
			else
			{
				MessageBox( hwnd, "Max scale resampling error: Invalid value", "sgexport - Error", MB_OK );
				break;
			}

			if ( getDlgItemTextFloat(hwnd, IDC_MAXSHADOWSAMPLEDISTANCE, &fv) )
			{
				exp->setMaxShadowSampleDistance( fv );
			}
			else
			{
				MessageBox( hwnd, "Max shadow sampling distance: Invalid value", "sgexport - Error", MB_OK );
				break;
			}

			// save options
			char cfg[1000];
			String cfgname = exp->name() + ".opt";
			cfgname.getBytes( cfg, sizeof(cfg), "ASCII-7" );
			FILE* fh = fopen( cfg, "wt" );
			writeBoolean( fh, exp->geometryExport() );
			writeBoolean( fh, exp->sceneExport() );
			writeBoolean( fh, exp->textureCopy() );
			writeBoolean( fh, exp->shadowOptimize() );
			writeFloat( fh, exp->maxShadowSampleDistance() );
			writeBoolean( fh, exp->forceDynamicShadows() );
			writeBoolean( fh, exp->collapsePatches() );
			writeBoolean( fh, exp->exportMorphTargets() );
			writeBoolean( fh, exp->compressLightMaps() );
			writeBoolean( fh, exp->staticLightingOnly() );
			writeBoolean( fh, exp->rescaleWorldUnits() );
			writeBoolean( fh, exp->resampleAnimations() );
			writeFloat( fh, AnimExportUtil::MAX_POSITION_RESAMPLING_ERROR );
			writeFloat( fh, AnimExportUtil::MAX_ROTATION_RESAMPLING_ERROR );
			writeFloat( fh, AnimExportUtil::MAX_SCALE_RESAMPLING_ERROR );
			if ( fh )
				fclose( fh );

			s_dialogReturn = 1;
			break;}

		case IDCANCEL:{
			s_dialogReturn = 0;
			break;}
		}
		break;}

	default:
		return FALSE;
	}
	return TRUE;
}       

/** Copies layer texture (if any) to the current working directory. */
static void copyLayerTexture( const GmMaterial::TextureLayer& layer )
{
	if ( layer.filename.length() > 0 )
		SceneExportUtil::copyFile( SceneExportUtil::stripPath(layer.filename), layer.filename );
}

//-----------------------------------------------------------------------------

MySceneExport::MySceneExport( HINSTANCE instance ) :
	m_models( Allocator< P(GmModel) >(__FILE__,__LINE__) ),
	m_shadows( Allocator< P(ShadowVolumeBuilder) >(__FILE__,__LINE__) ),
	m_allnodes( Allocator< P(SgNode) >(__FILE__,__LINE__) ),
	m_allnodes3ds( Allocator< INode* >(__FILE__,__LINE__) ),
	m_meshes( Allocator< P(SgMesh) >(__FILE__,__LINE__) ),
	m_lights( Allocator< P(SgLight) >(__FILE__,__LINE__) ),
	m_cameras( Allocator< P(SgCamera) >(__FILE__,__LINE__) ),
	m_lods( Allocator< P(SgLOD) >(__FILE__,__LINE__) ),
	m_dummies( Allocator< P(SgDummy) >(__FILE__,__LINE__) ),
	m_unknowns( Allocator< P(SgNode) >(__FILE__,__LINE__) )
{
	m_instance			= instance;
	m_progress			= 0;

	reset();
}

int	MySceneExport::ExtCount()
{
	return 1;
}

const TCHAR* MySceneExport::Ext( int i )
{
	switch ( i )
	{
	case 0:			return _T("sg");
	default:		return _T("");
	}
}

const TCHAR* MySceneExport::LongDesc()
{
	return _T("sgexport");
}

const TCHAR* MySceneExport::ShortDesc()
{
	return _T("sgexport");
}

const TCHAR* MySceneExport::AuthorName()
{
	return _T("");
}

const TCHAR* MySceneExport::CopyrightMessage()
{
	return _T("copyright 2001 (c) janik");
}

const TCHAR* MySceneExport::OtherMessage1()
{
	return _T("");
}

const TCHAR* MySceneExport::OtherMessage2()
{
	return _T("");
}

unsigned int MySceneExport::Version()
{
	return BUILD_NUMBER;
}

void MySceneExport::ShowAbout( HWND hwnd )
{
	char title[1000];
	sprintf( title, "About sgexport Build %i...", BUILD_NUMBER );
	
	char msg[1000];
	sprintf( msg, "%s\n", CopyrightMessage() );

	MessageBox( hwnd, msg, title, MB_OK );
}

int	MySceneExport::DoExport( const TCHAR* name, ExpInterface* ei, Interface* ip, BOOL quiet, DWORD options )
{
	try
	{
		reset();

		// make sure the name ends with .sg and not .SG
		m_name = name;
		if ( m_name.endsWith(".SG") )
			m_name = m_name.substring(0,m_name.length()-2) + "sg";

		// prompt the user for the options.
		if ( !quiet ) 
		{
			// init dialog
			HWND dlg = CreateDialogParam( m_instance, MAKEINTRESOURCE(IDD_EXPORT),
				ip->GetMAXHWnd(), exportDlgProc, (LPARAM)this );
			if ( !dlg )
				throw Exception( Format("Failed to create export dialog") );
			ShowWindow( dlg, SW_NORMAL );

			// message loop
			s_dialogReturn = -1;
			while( WinUtil::flushWindowMessages() && -1 == s_dialogReturn )
			{
				Sleep( 10 );
			}

			// destroy dialog
			DestroyWindow( dlg );
			if ( !s_dialogReturn )
				return 1;
		}

		// export and show progress
		m_progress = new ProgressDialog( m_instance, ip->GetMAXHWnd() );
		export( name, ei, ip, quiet, options );
		m_progress->destroy();

		// dump statistics
		Debug::println( "" );
		Debug::println( "Export profiling statistics:" );
		Debug::println( "----------------------------" );
		for ( int k = 0 ; k < Profile::count() ; ++k )
		{
			Profile::BlockInfo* b = Profile::get( k );
			Debug::println( "{0}: {1,#} times, {2,#.###} seconds", b->name(), b->count(), b->time() );
		}
		Debug::println( "" );

		reset();
	}
	catch ( const AbortExport& )
	{
	}
	catch ( const Throwable& err )
	{
		if ( m_progress )
			m_progress->destroy();

		if ( !quiet )
		{
			char buff[500];
			err.getMessage().format().getBytes( buff, sizeof(buff), "ASCII-7" );
			buff[sizeof(buff)-1] = 0;
			MessageBox( ip->GetMAXHWnd(), buff, "Error - sgexport", MB_OK|MB_ICONERROR );
		}
	}

	return 1;
}

void MySceneExport::export( const TCHAR* name, ExpInterface* ei, Interface* ip, BOOL noPrompts, DWORD options )
{
	Profile pr( "export" );

	Debug::println( "------------------------------------------------------------------------" );
	Debug::println( "sgexport Build {0}", BUILD_NUMBER );
	Debug::println( "------------------------------------------------------------------------" );
	bool rescaleDone = false;

	try
	{
		Debug::println( "Master scale: {0}m", GetMasterScale(UNITS_METERS) );
		if ( rescaleWorldUnits() )
		{
			Debug::println( "Rescaling system units to meters..." );
			ip->RescaleWorldUnits( GetMasterScale(UNITS_METERS), FALSE );
			rescaleDone = true;
		}

		m_progress->setText( "Preparing export..." );
		Debug::println( "Exporting 3DS Max scene to {0}", m_name );

		// exported animation range
		Interval animRange = ip->GetAnimRange();

		// If we export geometry then we also need to build
		// scene graph for shadow volume computations.
		// So exportScene() needs to be called before exportGeometry()
		exportScene( ip, animRange, options );
		exportGeometry( ip, animRange, options );

		if ( sceneExport() )
			writeSceneGraph();

		if ( geometryExport() )
			writeGeometry();

		if ( rescaleDone )
			ip->RescaleWorldUnits( 1.f / GetMasterScale(UNITS_METERS), FALSE );
	}
	catch ( ... )
	{
		if ( rescaleDone )
			ip->RescaleWorldUnits( 1.f / GetMasterScale(UNITS_METERS), FALSE );
		throw;
	}
}

BOOL MySceneExport::SupportsOptions( int ext, DWORD options )
{
	if ( 0 == ext )
	{
		if ( SCENE_EXPORT_SELECTED & options )
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}

bool MySceneExport::isExportableGeometry( INode* node, const ObjectState& os ) const
{
	require( node );
	require( os.obj );

	bool isShape = os.obj->SuperClassID() == SHAPE_CLASS_ID;
	bool isGeom = os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID;
	bool isTarget = os.obj->ClassID() == Class_ID(TARGET_CLASS_ID,0);
	bool isBiped = BipedUtil::isBiped(node);
	return isShape || ( isGeom && !isTarget && !isBiped );
}

void MySceneExport::exportScene( Interface* ip, Interval animRange, DWORD options )
{
	Profile pr( "exportScene" );

	// collect node to export
	Vector<INode*> nodes( Allocator<INode*>(__FILE__,__LINE__) );
	SceneExportUtil::getNodesToExport( ip, options, nodes );
	Debug::println( "" );
	Debug::println( "Nodes to be exported:" );
	Debug::println( "---------------------" );
	int k;
	for ( k = 0 ; k < nodes.size() ; ++k )
	{
		const TCHAR* name = nodes[k]->GetName();
		if ( !name )
			throw Exception( Format( "Scene contains unnamed nodes." ) );
		Debug::println( "{0}", name );
	}
	std::sort( nodes.begin(), nodes.end(), NodeNameLess() );

	// build scene graph
	Debug::println( "" );
	Debug::println( "Exporting scene graph..." );
	Debug::println( "------------------------" );

	m_allnodes.clear();
	m_allnodes3ds.clear();
	m_meshes.clear();
	m_lights.clear();
	m_cameras.clear();
	m_dummies.clear();
	m_unknowns.clear();
	m_lods.clear();
	
	for ( k = 0 ; k < nodes.size() ; ++k )
	{
		TimeValue	time			= 0;
		INode*		node			= nodes[k];
		TCHAR*		name			= node->GetName();

		// update progress
		m_progress->setProgress( float(k+1)/float(nodes.size()) );
		if ( name )
			m_progress->setText( Format("Exporting node {0}...", (String)name).format() );
		else
			m_progress->setText( "Exporting nodes..." );
		
		ObjectState os = node->EvalWorldState( time );
		if ( os.obj )
		{
			Debug::println( "Node {0} has world state", (String)name );
			if ( isExportableGeometry(node,os) )
			{
				Debug::println( "Node {0} is exportable geometry", (String)name );
				m_allnodes3ds.add( node );
				m_meshes.add( SgUtil::createMesh(node,animRange) );
				SgNode* obj = m_meshes.lastElement();
				require( dynamic_cast<SgMesh*>(obj) );
				m_allnodes.add( obj );
			}
			else if ( os.obj->SuperClassID() == LIGHT_CLASS_ID )
			{
				Debug::println( "Node {0} is light", (String)name );
				m_allnodes3ds.add( node );
				m_lights.add( SgUtil::createLight(node,animRange) );
				SgNode* obj = m_lights.lastElement();
				m_allnodes.add( obj );
			}
			else if ( os.obj->SuperClassID() == CAMERA_CLASS_ID )
			{
				Debug::println( "Node {0} is camera", (String)name );
				m_allnodes3ds.add( node );
				m_cameras.add( SgUtil::createCamera(node,animRange) );
				SgNode* obj = m_cameras.lastElement();
				m_allnodes.add( obj );
			}
			else if ( LODUtil::isLODHead(node) )
			{
				Debug::println( "Node {0} is level of detail root object", (String)name );
				m_allnodes3ds.add( node );
				m_lods.add( SgUtil::createLOD(node,animRange) );
				SgNode* obj = m_lods.lastElement();
				m_allnodes.add( obj );
			}
			else if ( os.obj->ClassID() == Class_ID(DUMMY_CLASS_ID,0) )
			{
				Debug::println( "Node {0} is dummy object", (String)name );
				m_allnodes3ds.add( node );
				m_dummies.add( SgUtil::createDummy(node,animRange) );
				SgNode* obj = m_dummies.lastElement();
				m_allnodes.add( obj );
			}
			else
			{
				Debug::println( "Node {0} is unknown object", (String)name );
				m_allnodes3ds.add( node );
				m_unknowns.add( SgUtil::createUnknown(node,animRange) );
				SgNode* obj = m_unknowns.lastElement();
				m_allnodes.add( obj );
			}
		}
	}
	require( m_allnodes.size() == m_allnodes3ds.size() );

	// DEBUG: print nodes
	Debug::println( "" );
	Debug::println( "Node indices:" );
	for ( k = 0 ; k < m_allnodes.size() ; ++k )
		Debug::println( "Node[{0}] = {1}", k, m_allnodes[k]->name );

	// find targets and parents
	m_progress->setText( "Connecting targets and parents..." );
	Debug::println( "" );
	Debug::println( "Connecting targets and parents..." );
	for ( k = 0 ; k < m_allnodes.size() ; ++k )
	{
		m_progress->setProgress( float(k+1)/float(m_allnodes.size()) );

		SgNode* obj = m_allnodes[k];
		INode* node = m_allnodes3ds[k];
		require( obj );
		require( node );
		require( (String)node->GetName() == obj->name );

		if ( !node->IsRootNode() )
		{
			INode* parent3ds = node->GetParentNode();

			// WORKAROUND: re-parent BipXX Spine to root instead of 
			// BipXX Pelvis to break dependency between lower body and upper body
			if ( BipedUtil::isBipedName(node->GetName(),"Spine") && parent3ds &&
				BipedUtil::isBipedName(parent3ds->GetName(),"Pelvis") )
			{
				parent3ds = parent3ds->GetParentNode();
				if ( !parent3ds )
					throw Exception( Format("{0} must have two parent levels", node->GetName()) );

				if ( parent3ds->IsRootNode() )
				{
					// hierarchy: (BipXX->BipXX Pelvis)->BipXX Spine
					//Debug::println( "BipXX is top-level object" );
					parent3ds = 0;
				}
				else
				{
					// hierarchy: MASTER_CTRL->(BipXX->BipXX Pelvis)->BipXX Spine
					parent3ds = parent3ds->GetParentNode();
					//Debug::println( "BipXX is parented to {0}", parent3ds->GetName() );
				}
			}

			obj->parent = m_allnodes3ds.indexOf( parent3ds );
			if ( -1 != obj->parent )
				obj->parentNode = m_allnodes[obj->parent];
		}

		if ( node->GetTarget() )
		{
			obj->target = m_allnodes3ds.indexOf( node->GetTarget() );
			if ( -1 != obj->target )
			{
				obj->targetNode = m_allnodes[obj->target];
				Debug::println( "  Node {0} is targeted to {1} (3ds node = {2}, index = {3})", (String)node->GetName(), obj->targetNode->name, (String)node->GetTarget()->GetName(), obj->target );
			}
		}
	}

	// add bones
	m_progress->setText( "Connecting bones..." );
	Debug::println( "" );
	Debug::println( "Adding bones..." );
	for ( k = 0 ; k < m_allnodes.size() ; ++k )
	{
		m_progress->setProgress( float(k+1)/float(m_allnodes.size()) );

		SgNode* obj = m_allnodes[k];
		SgMesh* mesh = dynamic_cast<SgMesh*>( obj );

		if ( mesh )
		{
			INode* node = m_allnodes3ds[k];
			require( obj );
			require( node );
			require( (String)node->GetName() == obj->name );

			SgUtil::addBones( mesh, node, m_allnodes3ds );
		}
	}

	// prepare lods
	m_progress->setText( "Preparing LODs..." );
	for ( int i = 0 ; i < m_meshes.size() ; ++i )
	{
		m_progress->setProgress( float(i+1)/float(m_meshes.size()) );

		SgMesh* mesh = m_meshes[i];
		mesh->lodNum = -1;

		if ( mesh->lodID != "" )
		{
			// reindex lod groups
			for ( int j = 0 ; j < m_lods.size() ; ++j )
			{
				if ( m_lods[j]->lodID == mesh->lodID )
					mesh->lodNum = j;
			}
			if ( -1 == mesh->lodNum )
				mesh->lodID = "";

			// find mesh lod minimum boundary
			mesh->lodMin = 0.f;
			for ( int j = 0 ; j < m_meshes.size() ; ++j )
			{
				SgMesh* other = m_meshes[j];
				if ( other->lodID == mesh->lodID )
				{
					if ( other->lodMax < mesh->lodMax &&
						other->lodMax > mesh->lodMin )
					{
						mesh->lodMin = other->lodMax;
					}
				}
			}
		}
	}

	// DEBUG: print node transforms etc.
	/*Matrix3 convtm = Matrix3( Point3(1,0,0), Point3(0,1,0), Point3(0,0,-1), Point3(0,0,0) );
	for ( int i = 0 ; i < m_allnodes.size() ; ++i )
	{
		INode* node = m_allnodes3ds[i];
		ObjectState os = node->EvalWorldState( 0 );
		if ( os.obj && os.obj->ClassID() == Class_ID(DUMMY_CLASS_ID,0) )
		{
			DummyObject* dummy = static_cast<DummyObject*>(os.obj);
			float x = (dummy->GetBox().pmax - dummy->GetBox().pmin).x;
			float y = (dummy->GetBox().pmax - dummy->GetBox().pmin).y;
			float z = (dummy->GetBox().pmax - dummy->GetBox().pmin).z;
			Debug::println( "Dummy object {0} size: {1} {2} {3}", (String)node->GetName(), x, y, z );
		}
		Matrix4x4 pm = TmUtil::toLH( TmUtil::getPivotTransform(node) * convtm );
		Matrix3x3 rot = pm.rotation();
		Debug::println( "{0} pivot scale: {1} {2} {3}", (String)node->GetName(), rot.getColumn(0).length(), rot.getColumn(1).length(), rot.getColumn(2).length() );
		Debug::println( "Object {0} node TM is", (String)node->GetName() );
		pm = TmUtil::getModelToParentLH( node, 0 );
		Debug::println( "  {0,#.###} {1,#.###} {2,#.###} {3,#.###}", pm(0,0), pm(0,1), pm(0,2), pm(0,3) );
		Debug::println( "  {0,#.###} {1,#.###} {2,#.###} {3,#.###}", pm(1,0), pm(1,1), pm(1,2), pm(1,3) );
		Debug::println( "  {0,#.###} {1,#.###} {2,#.###} {3,#.###}", pm(2,0), pm(2,1), pm(2,2), pm(2,3) );
		Debug::println( "  {0,#.###} {1,#.###} {2,#.###} {3,#.###}", pm(3,0), pm(3,1), pm(3,2), pm(3,3) );
		Matrix3 pivot = TmUtil::getPivotTransform( node );
		Debug::println( "object offs:" );
		Debug::println( "  {0,#.###} {1,#.###} {2,#.###}", pivot.GetRow(0).x, pivot.GetRow(0).y, pivot.GetRow(0).z );
		Debug::println( "  {0,#.###} {1,#.###} {2,#.###}", pivot.GetRow(1).x, pivot.GetRow(1).y, pivot.GetRow(1).z );
		Debug::println( "  {0,#.###} {1,#.###} {2,#.###}", pivot.GetRow(2).x, pivot.GetRow(2).y, pivot.GetRow(2).z );
		Debug::println( "  {0,#.###} {1,#.###} {2,#.###}", pivot.GetRow(3).x, pivot.GetRow(3).y, pivot.GetRow(3).z );
		Matrix3 nodetm = node->GetNodeTM( 0 );
		Debug::println( "object node tm:" );
		Debug::println( "  {0,#.###} {1,#.###} {2,#.###}", nodetm.GetRow(0).x, nodetm.GetRow(0).y, nodetm.GetRow(0).z );
		Debug::println( "  {0,#.###} {1,#.###} {2,#.###}", nodetm.GetRow(1).x, nodetm.GetRow(1).y, nodetm.GetRow(1).z );
		Debug::println( "  {0,#.###} {1,#.###} {2,#.###}", nodetm.GetRow(2).x, nodetm.GetRow(2).y, nodetm.GetRow(2).z );
		Debug::println( "  {0,#.###} {1,#.###} {2,#.###}", nodetm.GetRow(3).x, nodetm.GetRow(3).y, nodetm.GetRow(3).z );
	}*/

	// sample transformation animations
	m_progress->setText( "Sampling transformations..." );
	require( animRange.Start() <= animRange.End() );
	TimeValue dt = SGEXPORT_TICKS_PER_SAMPLE;
	// always resample whole animation ignoring current start frame
	for ( TimeValue t = 0/*animRange.Start()*/ ; t <= animRange.End() ; t += dt )
	{
		m_progress->setProgress( float(t)/float(animRange.End()) );

		for ( int i = 0 ; i < m_allnodes.size() ; ++i )
		{
			SgNode* node = m_allnodes[i];
			INode* node3ds = m_allnodes3ds[i];
			Matrix4x4 tm = TmUtil::getModelToParentLH( node3ds, t );
			node->tmAnim.add( tm );
		}
	}
	
	// resample transformation animations
	m_progress->setText( "Resampling transformations..." );
	require( animRange.Start() <= animRange.End() );
	for ( int i = 0 ; i < m_allnodes.size() ; ++i )
	{
		m_progress->setProgress( float(i+1)/float(m_allnodes.size()) );

		SgNode* node = m_allnodes[i];

		if ( node->resampleAnimations && !resampleAnimations() )
			node->resampleAnimations = resampleAnimations();

		node->resampleTransformAnimation( animRange, m_allnodes3ds[i] );
	}

	// get environment settings
	m_env = SceneEnvironment( ip, animRange );
}

void MySceneExport::exportGeometry( Interface* ip, Interval animRange, DWORD options )
{
	Profile pr( "exportGeometry" );

	Debug::println( "" );
	Debug::println( "Exporting geometry..." );
	Debug::println( "---------------------" );
	
	// export geometry from 3ds nodes
	m_models.clear();
	for ( int k = 0 ; k < m_allnodes3ds.size() ; ++k )
	{
		INode*		node3ds			= m_allnodes3ds[k];
		SgNode*		node			= m_allnodes[k];
		String		name			( node3ds->GetName() );
		Mtl*		mat				= node3ds->GetMtl();

		// update progress
		m_progress->setProgress( float(k+1)/float(m_allnodes3ds.size()) );
		m_progress->setText( Format("Exporting geometry {0}...", name).format() );

		ObjectState os = node3ds->EvalWorldState( 0 );
		if ( os.obj )
		{
			if ( isExportableGeometry(node3ds,os) )
			{
				// SgNode should be SgMesh
				SgMesh* mesh = dynamic_cast<SgMesh*>( node );
				if ( !mesh )
					Debug::println( "Node {0} is not a mesh", node->name );
				require( mesh );

				// get geometry data to export
				Mesh* trimesh = 0;
				PatchMesh* patchmesh = 0;
				std::auto_ptr<TriObject> triAutoDel(0);
				std::auto_ptr<PatchObject> patchAutoDel(0);
				ShapeObject* shape = 0;
				// we have to test against absolute class since
				// Max can implicitly convert EditableMesh to PatchObject if
				// any vertices haven't been modified...
				if ( os.obj->ClassID() == Class_ID(PATCHOBJ_CLASS_ID,0) )
				{
					PatchObject* patch = static_cast<PatchObject*>( os.obj->ConvertToType( 0, Class_ID(PATCHOBJ_CLASS_ID,0) ) );
					if ( patch != os.obj )
						patchAutoDel = std::auto_ptr<PatchObject>( patch );
					patchmesh = &patch->patch;
					Debug::println( "{0} is PatchObject ({1} patches)", name, patchmesh->numPatches );
				}
				if ( os.obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)) )
				{
					if ( !patchmesh || collapsePatches() )
					{
						TriObject* tri = static_cast<TriObject*>( os.obj->ConvertToType( 0, Class_ID(TRIOBJ_CLASS_ID,0) ) );
						if ( tri != os.obj )
							triAutoDel = std::auto_ptr<TriObject>( tri );
						trimesh = &tri->mesh;
						Debug::println( "Collapsed {0} to TriObject ({1} polys, {2} verts)", name, trimesh->numFaces, trimesh->numVerts );
					}
				}
				if ( os.obj->SuperClassID() == SHAPE_CLASS_ID )
				{
					shape = static_cast<ShapeObject*>( os.obj );
					Debug::println( "{0} is ShapeObject", name );
				}

				// check that we have material defined
				if ( !mat && !shape )
					Debug::printlnWarning( "Object {0} has not material set", name );

				// extract geometry data
				if ( trimesh || patchmesh || shape )
				{
					P(GmModel) gm = GmUtil::createGmModel( node3ds, trimesh, patchmesh, shape, mat );

					// check if the geometry has already been exported
					GmModel* model = 0;
					/* DOESN'T WORK WITH MORPHERS!! (heads with different morphers get instanced)
					if ( !staticLightingOnly() )
					{
						for ( int j = 0 ; j < m_models.size() ; ++j )
						{
							GmModel* geom = m_models[j];
							if ( geom->filename == gm->filename || *geom == *gm )
							{
								Debug::println( "{0} is instance of {1}", gm->name, geom->name );
								model = geom;
								break;
							}
						}
					}*/

					if ( !model )
					{
						model = gm;
						m_models.add( gm );
					}

					require( mesh );
					mesh->model = model;
				}
			}
		}
	}

	// export morphs
	m_progress->setText( "Exporting morphs..." );
	Vector<float> weightAnimFrames( Allocator<float>(__FILE__) );
	KeyFrameContainer weightAnim( 1, KeyFrame::DATA_SCALAR );
	for ( int i = 0 ; i < m_models.size() ; ++i )
	{
		m_progress->setProgress( float(i+1)/float(m_models.size()) );

		GmModel* gm = m_models[i];
		require( gm );
		require( gm->node3ds );

		MorphR3* mod = MorphUtil::getMorpherModifier( gm->node3ds );
		if ( mod )
		{
			Debug::println( "MorphR3 found from {0}", gm->name );

			for ( int k = 0 ; k < mod->chanBank.size() ; ++k )
			{
				// valid morph target?
				morphChannel& chn = mod->chanBank[k];
				if ( chn.mActive && !chn.mInvalid )
				{
					// find corresponding gm
					if ( !chn.mConnection )
						throw Exception( Format("Morph channel {0} has lost its source object!", String(chn.mName)) );
					require( chn.mName );
					require( chn.mConnection );
					Debug::println( "  Morph channel {0} is active, finding connection {1}", String(chn.mName), String(chn.mConnection->GetName()) );
					GmModel* target = 0;
					for ( int j = 0 ; j < m_models.size() ; ++j )
					{
						if ( m_models[j]->node3ds == chn.mConnection )
						{
							target = m_models[j];
							break;
						}
					}
					require( target );
					
					// mark gm as morph target
					target->morphBase = gm;
					String chnName( chn.mName );

					// extract weight animation
					weightAnim.clear();
					MorphUtil::getWeightAnimation( chn, animRange, &weightAnimFrames );
					AnimExportUtil::resampleFloatAnimation( weightAnimFrames, animRange, &weightAnim, AnimExportUtil::MAX_MORPH_RESAMPLING_ERROR/100.f );
					AnimExportUtil::offsetKeyTimes( &weightAnim, TicksToSec(animRange.Start()) );
					Debug::println( "  Channel {0} animation ({1}): {2} keys", chnName, target->name, weightAnim.keys() );

					// add morph channel to this morpher
					P(GmMorphChannel) morphchn = new GmMorphChannel( chnName, weightAnim, target );
					gm->morphChannels.add( morphchn );
				}
			}
		}
	}

	// split geometry to primitives
	m_progress->setText( "Processing discontinuous geometry data..." );
	for ( int i = 0 ; i < m_models.size() ; ++i )
	{
		m_progress->setProgress( float(i+1)/float(m_models.size()) );
		GmModel* gm = m_models[i];
		require( gm );
		gm->splitToPrimitives();
	}

	// compute vertex lights to model primitives 
	// if static lighting selected and model primitive has no lightmaps
	if ( staticLightingOnly() )
	{
		m_progress->setText( "Computing static vertex lighting..." );

		Vector<Vector3> vertexWorldPositions( Allocator<Vector3>(__FILE__) );
		Vector<Vector3> vertexWorldNormals( Allocator<Vector3>(__FILE__) );
		Vector<Vector3> vertexDiffuseColors( Allocator<Vector3>(__FILE__) );
		Vector<SgLight*> lights( Allocator<SgLight*>(__FILE__) );

		for ( int m = 0 ; m < m_meshes.size() ; ++m )
		{
			m_progress->setProgress( float(m+1)/float(m_meshes.size()) );
			SgMesh* mesh = m_meshes[m];
			Matrix4x4 meshtm = mesh->getWorldTransform( 0.f );
			GmModel* gm = mesh->model;
			mb::DiscontinuousVertexMap* rgb = gm->addDiscontinuousVertexMap( 3, gm->name + " : RGB", mb::VertexMapFormat::VERTEXMAP_RGB );
			require( gm );

			for ( int k = 0 ; k < gm->primitives.size() ; ++k )
			{
				GmModelPrimitive* prim = gm->primitives[k];
				if ( 0 == (prim->mat->layerFlags() & GmMaterial::L_LGHT) &&
					prim->mat->selfIllum < 0.99f )
				{
					const int verts = prim->vertices.size();

					// get vertex world positions and normals
					vertexWorldPositions.setSize( 0 );
					vertexWorldNormals.setSize( 0 );
					for ( int j = 0 ; j < verts ; ++j )
					{
						mb::Vertex* vert = prim->vertices[j];
						require( vert );
						Vector3 localPos;
						vert->getPosition( &localPos.x, &localPos.y, &localPos.z );
						Vector3 localNormal;
						require( prim->normals );
						prim->normals->getValue( vert->index(), vert->getPolygon(0)->index(), &localNormal.x, 3 );
						Vector3 worldPos = meshtm.transform( localPos );
						Vector3 worldNormal = meshtm.rotate( localNormal );

						vertexWorldPositions.add( worldPos );
						vertexWorldNormals.add( worldNormal );
					}

					// sorts lights by ascending distance to mesh
					lights.setSize( 0 );
					for ( int l = 0 ; l < m_lights.size() ; ++l )
					{
						SgLight* lt = m_lights[l];
						if ( lt->name.indexOf("VLIT") >= 0 )
						{
							lt->tempDistance = (lt->getWorldTransform(0.f).translation() - meshtm.translation()).length();
							lights.add( lt );
						}
					}
					if ( lights.size() == 0 )
						throw Exception( Format("Tried to compute static vertex lighting but no lights with VLIT name tag present in the scene") );
					std::sort( lights.begin(), lights.end(), SgLightDistanceLess() );

					// apply lights
					vertexDiffuseColors.clear();
					vertexDiffuseColors.setSize( verts, Vector3(0,0,0) );
					for ( int l = 0 ; l < lights.size() ; ++l )
						lights[l]->lit( vertexWorldPositions.begin(), vertexWorldNormals.begin(), vertexDiffuseColors.begin(), verts );
					
					// store results
					for ( int j = 0 ; j < verts ; ++j )
						rgb->addValue( prim->vertices[j]->index(), prim->vertices[j]->getPolygon(0)->index(), &vertexDiffuseColors[j].x, 3 );
					
					prim->vertcolor = rgb;
					prim->mat->selfIllum = 1.f; // disable lighting
				}
			}
		}
	}

	// compress light map textures if requested
	if ( compressLightMaps() )
	{
		m_progress->setText( "Compressing textures..." );
		for ( int i = 0 ; i < m_models.size() ; ++i )
		{
			m_progress->setProgress( float(i+1)/float(m_models.size()) );
			GmModel* gm = m_models[i];
			require( gm );

			Vector<lang::String> filenames( Allocator<lang::String>(__FILE__,__LINE__) );

			for ( int j = 0 ; j < gm->materials.size() ; ++j )
			{
				GmMaterial* mat = gm->materials[j];
				
				if ( mat->lightMapLayer.filename.length() > 0 ) // Check for light map
				{
					filenames.add( mat->lightMapLayer.filename );
		
					// Change file extension to .dds

					int extensionoffset = mat->lightMapLayer.filename.lastIndexOf( "." );

					assert( extensionoffset != -1 ); 
					if ( extensionoffset != - 1)
						mat->lightMapLayer.filename = mat->lightMapLayer.filename.substring(0, extensionoffset) + String(".dds");
					else
						Debug::printlnError( "Extension was not found in lighting map file name, compression failed." );
				}
			}
			std::sort( filenames.begin(), filenames.end() );
			filenames.setSize( std::unique( filenames.begin(), filenames.end() ) - filenames.begin() );
			
			for ( int j = 0; j < filenames.size(); ++j )
			{
				String filename = String("\"") + filenames[j] + String("\"");
				String path = String("\"") + SceneExportUtil::getPath( filenames[j] ) + String("\"");

				char fn[256];
				char pt[256];

				filename.getBytes( fn, sizeof(fn), "ASCII-7" );
				path.getBytes( pt, sizeof(pt), "ASCII-7" );

				// Execute compression program
				char* args[8];
				args[0] = "nvdxt.exe";
				args[1] = "-file";
				args[2] = fn;
				args[3] = "-dxt1c";
				args[4] = "-full";
				args[5] = "-outdir";				
				args[6] = pt;	
				args[7] = 0;

				_flushall();
				int err = _spawnvp( _P_WAIT, "nvdxt.exe", args );
				
				if ( err == -1 )
					throw Exception( Format("Spawn of nvdxt.exe failed!") );

			}
		}
	}

	// copy textures to work dir if requested
	if ( textureCopy() )
	{
		m_progress->setText( "Copying textures..." );
		for ( int i = 0 ; i < m_models.size() ; ++i )
		{
			m_progress->setProgress( float(i+1)/float(m_models.size()) );
			GmModel* gm = m_models[i];
			require( gm );

			for ( int j = 0 ; j < gm->materials.size() ; ++j )
			{
				GmMaterial* mat = gm->materials[j];
				copyLayerTexture( mat->diffuseLayer );
				copyLayerTexture( mat->reflectionLayer );
				copyLayerTexture( mat->bumpLayer );
				copyLayerTexture( mat->lightMapLayer );
				copyLayerTexture( mat->specularColorLayer );
				copyLayerTexture( mat->specularLevelLayer );
			}
		}
	}

	// find keylight
	SgLight* keylight = 0;
	for ( int i = 0 ; i < m_lights.size() ; ++i )
	{
		if ( m_lights[i]->name == "keylight" )
		{
			keylight = m_lights[i];
			break;
		}
	}

	// compute mesh bounding boxes
	m_progress->setText( "Computing bounding volumes..." );
	for ( int i = 0 ; i < m_meshes.size() ; ++i )
	{
		m_progress->setProgress( float(i+1)/float(m_meshes.size()) );

		SgMesh* mesh = m_meshes[i];
		require( mesh );

		Vector3 p;
		OBBoxBuilder obbb;
		while ( obbb.nextPass() )
		{
			require( mesh->model );
			for ( int k = 0 ; k < mesh->model->vertices() ; ++k )
			{
				mesh->model->getVertex( k )->getPosition( &p.x, &p.y, &p.z );
				obbb.addPoints( &p, 1 );
			}
		}

		mesh->obb = obbb.box();
	}

	// build shadow volumes
	m_shadows.clear();
	if ( keylight || forceDynamicShadows() )
	{
		for ( int i = 0 ; i < m_meshes.size() ; ++i )
		{
			require( m_meshes[i]->model );
			SgMesh* mesh = m_meshes[i];

			m_progress->setText( Format("Exporting shadows {0}...", mesh->name).format() );
			m_progress->setProgress( float(i+1)/float(m_meshes.size()) );

			if ( mesh->castShadows && mesh->model && 
				mesh->model->polygons() > 0 && 
				mesh->model->vertices() >= 3 )
			{
				P(ShadowVolumeBuilder) shadow = new ShadowVolumeBuilder( mesh, keylight, forceDynamicShadows() );
				mesh->shadow = shadow;
				m_shadows.add( shadow );
			}
		}

		if ( shadowOptimize() && keylight )
		{
			ShadowVolumeBuilder::optimize( m_meshes, m_progress, maxShadowSampleDistance(),
				keylight->getWorldTransform(0).rotation().getColumn(2).normalize() );
		}
	}
}

void MySceneExport::writeGeometry()
{
	Profile pr( "writeGeometry" );

	m_progress->setText( "Writing geometry..." );
	Debug::println( "" );
	Debug::println( "Writing geometry..." );
	Debug::println( "-------------------" );

	// write the geometry
	for ( int i = 0 ; i < m_models.size() ; ++i )
	{
		GmModel* gm = m_models[i];
		require( gm );

		m_progress->setProgress( float(i+1)/float(m_models.size()) );

		if ( exportMorphTargets() || !gm->morphBase )
		{
			Debug::println( "" );
			Debug::println( "Writing {0}", gm->filename );

			FileOutputStream file( gm->filename );
			ChunkOutputStream out( &file );
			gm->write( &out );
			out.close();
			file.close();
		}
	}
}

void MySceneExport::writeSceneGraph()
{
	Profile pr( "writeSceneGraph" );

	m_progress->setText( "Writing scene graph..." );
	Debug::println( "" );
	Debug::println( "Writing scene graph..." );
	Debug::println( "----------------------" );

	// open scene graph (for writing)
	FileOutputStream file( m_name );
	ChunkOutputStream out( &file );
	out.beginChunk( "sg" );
	out.writeInt( SG_FILE_VER );

	// write rendering environment
	m_env.write( &out );

	// write all nodes
	for ( int k = 0 ; k < m_allnodes.size() ; ++k )
	{
		m_progress->setProgress( float(k+1)/float(m_allnodes.size()) );
		m_allnodes[k]->write( &out );
	}

	// close scene graph
	out.endChunk();
	out.close();
	file.close();
}

void MySceneExport::convertLightMapsToDxt()
{
	
}

void MySceneExport::reset()
{
	m_progress = 0;

	m_name						= "";
	m_geometryExport			= true;
	m_sceneExport				= true;
	m_resampleAnimations		= true;
	m_textureCopy				= true;
	m_shadowOptimize			= true;
	m_forceDynamicShadow		= false;
	m_collapsePatches			= true;
	m_exportMorphTargets		= true;
	m_compressLightMaps			= false;
	m_staticLightingOnly		= false;
	m_rescaleWorldUnits			= false;
	m_maxShadowSampleDistance	= 10.f;

	m_models.clear();
	m_shadows.clear();
	m_allnodes.clear();
	m_allnodes3ds.clear();
	m_meshes.clear();
	m_lights.clear();
	m_cameras.clear();
	m_lods.clear();
	m_dummies.clear();
	m_unknowns.clear();

	m_env	= SceneEnvironment();

	Profile::reset();
}
