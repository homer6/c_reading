#include "SgNode.h"
#include "SgMesh.h"
#include "SgLight.h"
#include "SgCamera.h"
#include "SgLOD.h"
#include "SgDummy.h"
#include "GmModel.h"
#include "SceneEnvironment.h"
#include "ShadowVolumeBuilder.h"
#include "ProgressDialog.h"
#include <lang/String.h>
#include <util/Vector.h>


namespace io {
	class DataOutputStream;}


/**
 * Main export class.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class MySceneExport :
	public SceneExport
{
public:
	explicit MySceneExport( HINSTANCE instance );

	int					ExtCount();
	const TCHAR*		Ext( int i );
	const TCHAR*		LongDesc();
	const TCHAR*		ShortDesc();
	const TCHAR*		AuthorName();
	const TCHAR*		CopyrightMessage();
	const TCHAR*		OtherMessage1();
	const TCHAR*		OtherMessage2();
	unsigned int		Version();
	void				ShowAbout( HWND hwnd );
	int					DoExport( const TCHAR* name, ExpInterface* ei, Interface* i, BOOL quiet, DWORD options=0 );
	BOOL				SupportsOptions( int ext, DWORD options );

	/** Enables/disables geometry export. */
	void				setGeometryExport( bool enabled )							{m_geometryExport=enabled;}
	
	/** Enables/disables scene export. */
	void				setSceneExport( bool enabled )								{m_sceneExport=enabled;}

	/** Enables/disables animation resampling. */
	void				setResampleAnimations( bool enabled )						{m_resampleAnimations=enabled;}
	
	/** Enables/disables texture copying. */
	void				setTextureCopy( bool enabled )								{m_textureCopy=enabled;}

	/** Enables/disables shadow optimization. */
	void				setShadowOptimize( bool enabled )							{m_shadowOptimize=enabled;}

	/** Enables/disables forcing of dynamic shadows. */
	void				setForceDynamicShadows( bool enabled )						{m_forceDynamicShadow=enabled;}

	/** Enables/disables patch collapsing to triangle meshes. */
	void				setCollapsePatches( bool enabled ) 							{m_collapsePatches=enabled;}

	/** Enables/disables morph target exporting. */
	void				setExportMorphTargets( bool enabled ) 						{m_exportMorphTargets=enabled;}

	/** Enables/disables light map compressing. */
	void				setCompressLightMaps( bool enabled )						{m_compressLightMaps=enabled;}

	/** Enables/disables static lighting only. */
	void				setStaticLightingOnly( bool enabled )						{m_staticLightingOnly=enabled;}

	/** Set true if world units are scaled to meters. */
	void				setRescaleWorldUnits( bool enabled )						{m_rescaleWorldUnits=enabled;}

	/** Sets maximum distance between two shadow rays on same edge. */
	void				setMaxShadowSampleDistance( float dist ) 					{m_maxShadowSampleDistance = dist;}

	/** Returns name of the current scene file to be exported. */
	const lang::String&	name() const												{return m_name;}

	/** Returns true if geometry exporting is enabled. */
	bool				resampleAnimations() const									{return m_resampleAnimations;}

	/** Returns true if geometry exporting is enabled. */
	bool				geometryExport() const										{return m_geometryExport;}

	/** Returns true if scene exporting is enabled. */
	bool				sceneExport() const											{return m_sceneExport;}

	/** Returns true if texture copying is enabled. */
	bool				textureCopy() const											{return m_textureCopy;}

	/** Returns true if shadow optimization is enabled. */
	bool				shadowOptimize() const										{return m_shadowOptimize;}

	/** Returns true if forcing of dynamic shadows is enabled. */
	bool				forceDynamicShadows() const									{return m_forceDynamicShadow;}

	/** Returns true if patches are automatically collapsed to triangle meshes. */
	bool				collapsePatches() const										{return m_collapsePatches;}

	/** Returns true if morph targets are exported. */
	bool				exportMorphTargets() const									{return m_exportMorphTargets;}

	/** Returns true if lightmaps should be compressed. */
	bool				compressLightMaps() const									{return m_compressLightMaps;}

	/** Returns true if static lighting only. */
	bool				staticLightingOnly() const									{return m_staticLightingOnly;}

	/** Returns true if world units are scaled to meters. */
	bool				rescaleWorldUnits() const									{return m_rescaleWorldUnits;}

	/** Returns maximum distance between two shadow rays on same edge. */
	float				maxShadowSampleDistance() const								{return m_maxShadowSampleDistance;}

	/** Tests if specified node/object state is exportable geometry. */
	bool				isExportableGeometry( INode* node, const ObjectState& os ) const;

private:
	HINSTANCE							m_instance;
	P(ProgressDialog)					m_progress;

	lang::String						m_name;
	bool								m_geometryExport;
	bool								m_sceneExport;
	bool								m_resampleAnimations;
	bool								m_textureCopy;
	bool								m_shadowOptimize;
	bool								m_forceDynamicShadow;
	bool								m_collapsePatches;
	bool								m_exportMorphTargets;
	bool								m_compressLightMaps;
	bool								m_staticLightingOnly;
	bool								m_rescaleWorldUnits;
	float								m_maxShadowSampleDistance;

	util::Vector< P(GmModel) >				m_models;
	util::Vector< P(ShadowVolumeBuilder) >	m_shadows;
	util::Vector< P(SgNode) >				m_allnodes;
	util::Vector< INode* >					m_allnodes3ds;
	util::Vector< P(SgMesh) >				m_meshes;
	util::Vector< P(SgLight) >				m_lights;
	util::Vector< P(SgCamera) >				m_cameras;
	util::Vector< P(SgLOD) >				m_lods;
	util::Vector< P(SgDummy) >				m_dummies;
	util::Vector< P(SgNode) >				m_unknowns;
	SceneEnvironment						m_env;

	void	reset();
	void	export( const TCHAR* name, ExpInterface* ei, Interface* i, BOOL quiet, DWORD options );
	void	exportScene( Interface* ip, Interval animRange, DWORD options );
	void	exportGeometry( Interface* ip, Interval animRange, DWORD options );
	void	writeGeometry();
	void	writeSceneGraph();
	void	convertLightMapsToDxt();

	MySceneExport( const MySceneExport& );
	MySceneExport& operator=( const MySceneExport& );
};
