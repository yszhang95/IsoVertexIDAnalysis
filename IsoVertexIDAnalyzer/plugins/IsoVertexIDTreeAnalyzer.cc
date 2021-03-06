// -*- C++ -*-
//
// Package:    IsoVertexIDAnalysis/IsoVertexIDTreeAnalyzer
// Class:      IsoVertexIDTreeAnalyzer
// 
/**\class IsoVertexIDTreeAnalyzer IsoVertexIDTreeAnalyzer.cc IsoVertexIDAnalysis/IsoVertexIDTreeAnalyzer/plugins/IsoVertexIDTreeAnalyzer.cc

 Description: Vertex tree producer

 Implementation:
     
*/
//
// Original Author:  Wei Li
//         Created:  Thu, 19 Dec 2019 17:42:08 GMT
//
//


// system include files
#include <memory>
#include <exception>
#include <iostream>
#include <memory>
#include <math.h>
#include <vector>
#include <string>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/Math/interface/deltaPhi.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "SimDataFormats/Vertex/interface/SimVertex.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"
#include "SimGeneral/HepPDTRecord/interface/ParticleDataTable.h"
#include "FWCore/Common/interface/TriggerNames.h"

#include <TTree.h>

//
// class declaration
//

class TTree;

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<> and also remove the line from
// constructor "usesResource("TFileService");"
// This will improve performance in multithreaded jobs.

#define NMAXVTX 100
#define NMAXTRACKSVTX 300

class IsoVertexIDTreeAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
   public:
      explicit IsoVertexIDTreeAnalyzer(const edm::ParameterSet&);
      ~IsoVertexIDTreeAnalyzer();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

   private:

      edm::Service<TFileService> theOutputs;
      edm::EDGetTokenT<reco::TrackCollection> token_tracks;
      edm::EDGetTokenT<reco::VertexCollection> token_vertices;

      TTree* vertexTree;

      uint runNb;
      uint eventNb;
      uint lsNb;

      bool isSlim_;

      uint nVertices;
      uint nTracks[NMAXVTX];

      float xVtx[NMAXVTX];
      float yVtx[NMAXVTX];
      float zVtx[NMAXVTX];
      float xVtxErr[NMAXVTX];
      float yVtxErr[NMAXVTX];
      float zVtxErr[NMAXVTX];
      float chi2Vtx[NMAXVTX];
      int   ndofVtx[NMAXVTX];
      float trackWeightVtx[NMAXVTX][NMAXTRACKSVTX];
      float trackPtVtx[NMAXVTX][NMAXTRACKSVTX];
      float trackEtaVtx[NMAXVTX][NMAXTRACKSVTX];
      float trackPhiVtx[NMAXVTX][NMAXTRACKSVTX];
      float trackPtErrVtx[NMAXVTX][NMAXTRACKSVTX];
      float trackXVtx[NMAXVTX][NMAXTRACKSVTX];
      float trackYVtx[NMAXVTX][NMAXTRACKSVTX];
      float trackZVtx[NMAXVTX][NMAXTRACKSVTX];
      float trackXYErrVtx[NMAXVTX][NMAXTRACKSVTX];
      float trackZErrVtx[NMAXVTX][NMAXTRACKSVTX];
      bool  trackHPVtx[NMAXVTX][NMAXTRACKSVTX];

      void resetArrays();
      virtual void beginJob() override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;

      // ----------member data ---------------------------
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
IsoVertexIDTreeAnalyzer::IsoVertexIDTreeAnalyzer(const edm::ParameterSet& iConfig):
isSlim_(iConfig.getParameter<bool>("isSlim"))
{
   //now do what ever initialization is needed
   usesResource("TFileService");

   token_vertices = consumes<std::vector<reco::Vertex>>(iConfig.getParameter<edm::InputTag>("VertexCollection"));
   token_tracks = consumes<std::vector<reco::Track>>(iConfig.getParameter<edm::InputTag>("TrackCollection"));
}


IsoVertexIDTreeAnalyzer::~IsoVertexIDTreeAnalyzer()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called for each event  ------------
void
IsoVertexIDTreeAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   resetArrays();

   using namespace edm;

   edm::Handle< reco::VertexCollection > vertices;
   iEvent.getByToken(token_vertices, vertices);
   if(!vertices->size()) { std::cout<<"Invalid or empty vertex collection!"<<std::endl; vertexTree->Fill(); return; }

/*
   edm::Handle< reco::TrackCollection > tracks;
   iEvent.getByToken(token_tracks, tracks);
   if(!tracks->size()) { std::cout<<"Invalid or empty track collection!"<<std::endl; return; }
*/

   runNb = iEvent.id().run();
   eventNb = iEvent.id().event();
   lsNb = iEvent.luminosityBlock();

   nVertices=0;

//std::cout<<vertices->size()<<std::endl;

   for(unsigned int iv=0; iv<vertices->size(); iv++)
   {
     const reco::Vertex & vtx = (*vertices)[iv];

     if(!vtx.isFake() && vtx.tracksSize()>=2)
     {
       xVtx[nVertices] = vtx.x();
       yVtx[nVertices] = vtx.y();
       zVtx[nVertices] = vtx.z();
       xVtxErr[nVertices] = vtx.xError();
       yVtxErr[nVertices] = vtx.yError();
       zVtxErr[nVertices] = vtx.zError();
       chi2Vtx[nVertices] = vtx.chi2();
       ndofVtx[nVertices] = vtx.ndof();

       uint nTracksTmp=0;
       for (reco::Vertex::trackRef_iterator iTrack = vtx.tracks_begin(); iTrack != vtx.tracks_end(); iTrack++) {

          reco::TrackRef track = iTrack->castTo<reco::TrackRef>();

          trackWeightVtx[nVertices][nTracksTmp] = vtx.trackWeight(*iTrack);
          trackPtVtx[nVertices][nTracksTmp] = track->pt();
          trackEtaVtx[nVertices][nTracksTmp] = track->eta();
          trackPhiVtx[nVertices][nTracksTmp] = track->phi();
          trackPtErrVtx[nVertices][nTracksTmp] = track->ptError();
          trackXVtx[nVertices][nTracksTmp] = track->vx();
          trackYVtx[nVertices][nTracksTmp] = track->vy();
          trackZVtx[nVertices][nTracksTmp] = track->vz();
          trackXYErrVtx[nVertices][nTracksTmp] = track->d0Error();
          trackZErrVtx[nVertices][nTracksTmp] = track->dzError();
          trackHPVtx[nVertices][nTracksTmp] = track->quality(reco::TrackBase::highPurity);
//std::cout<<nVertices<<" "<<zVtx[nVertices]<<" "<<nTracksTmp<<" "<<trackPtVtx[nVertices][nTracksTmp]<<" "<<trackZVtx[nVertices][nTracksTmp]<<std::endl;

          if(isSlim_)
          {
            if(!trackHPVtx[nVertices][nTracksTmp]) continue;
            if(trackPtErrVtx[nVertices][nTracksTmp]/trackPtVtx[nVertices][nTracksTmp]>0.1) continue;
            if(fabs(trackZVtx[nVertices][nTracksTmp]-zVtx[nVertices])/pow(trackZErrVtx[nVertices][nTracksTmp]*trackZErrVtx[nVertices][nTracksTmp]+zVtxErr[nVertices]*zVtxErr[nVertices],0.5)>3) continue;
            if(pow(pow(trackXVtx[nVertices][nTracksTmp]-xVtx[nVertices],2)+pow(trackYVtx[nVertices][nTracksTmp]-yVtx[nVertices],2),0.5)/pow(trackXYErrVtx[nVertices][nTracksTmp]*trackXYErrVtx[nVertices][nTracksTmp]+xVtxErr[nVertices]*yVtxErr[nVertices],0.5)>3) continue;             
          }
          nTracksTmp++;
       }
       nTracks[nVertices] = nTracksTmp;

       nVertices++;
     }
   }
/*
   for(uint i=0;i<nVertices;i++)
   {
     for(uint j=0;j<nTracks[i];j++)
     {
std::cout<<"After: "<<i<<" "<<zVtx[i]<<" "<<j<<" "<<trackPtVtx[i][j]<<" "<<trackZVtx[i][j]<<std::endl;

     }
   }
*/
   vertexTree->Fill();
}

void
IsoVertexIDTreeAnalyzer::resetArrays()
{
  for(int i=0;i<NMAXVTX;i++)
  {
    xVtx[i] = -999.0;
    yVtx[i] = -999.0;
    zVtx[i] = -999.0;
    xVtxErr[i] = -999.0;
    yVtxErr[i] = -999.0;
    zVtxErr[i] = -999.0;
    chi2Vtx[i] = -999.0;
    ndofVtx[i] = -999.0;
    nTracks[i] = -999;
    
    for(int j=0;j<NMAXTRACKSVTX;j++)
    {
      trackWeightVtx[i][j] = -999.0;
      trackPtVtx[i][j] = -999.0;
      trackEtaVtx[i][j] = -999.0;
      trackPhiVtx[i][j] = -999.0;
      trackPtErrVtx[i][j] = -999.0;
      trackXVtx[i][j] = -999.0;
      trackYVtx[i][j] = -999.0;
      trackZVtx[i][j] = -999.0;
      trackXYErrVtx[i][j] = -999.0;
      trackZErrVtx[i][j] = -999.0;
      trackHPVtx[i][j] = -999.0;
    }
  }
}

// ------------ method called once each job just before starting event loop  ------------
void 
IsoVertexIDTreeAnalyzer::beginJob()
{
  vertexTree = theOutputs->make<TTree>("vertexTree","vertexTree");
  vertexTree->Branch("RunNb",&runNb,"RunNb/i");
  vertexTree->Branch("LSNb",&lsNb,"LSNb/i");
  vertexTree->Branch("EventNb",&eventNb,"EventNb/i");
  vertexTree->Branch("nVertices",&nVertices,"nVertices/i");
  vertexTree->Branch("xVtx",xVtx,"xVtx[nVertices]/F");
  vertexTree->Branch("yVtx",yVtx,"yVtx[nVertices]/F");
  vertexTree->Branch("zVtx",zVtx,"zVtx[nVertices]/F");
  vertexTree->Branch("nTracks",nTracks,"nTracks[nVertices]/i");
  vertexTree->Branch("trackPtVtx",trackPtVtx,"trackPtVtx[nVertices][300]/F");
  vertexTree->Branch("trackEtaVtx",trackEtaVtx,"trackEtaVtx[nVertices][300]/F");
  vertexTree->Branch("trackPhiVtx",trackPhiVtx,"trackPhiVtx[nVertices][300]/F");

  if(!isSlim_)
  {
    vertexTree->Branch("xVtxErr",xVtxErr,"xVtxErr[nVertices]/F");
    vertexTree->Branch("yVtxErr",yVtxErr,"yVtxErr[nVertices]/F");
    vertexTree->Branch("zVtxErr",zVtxErr,"zVtxErr[nVertices]/F");
    vertexTree->Branch("chi2Vtx",chi2Vtx,"chi2Vtx[nVertices]/F");
    vertexTree->Branch("ndofVtx",ndofVtx,"ndofVtx[nVertices]/i");
    vertexTree->Branch("trackWeightVtx",trackWeightVtx,"trackWeightVtx[nVertices][300]/F");
    vertexTree->Branch("trackPtErrVtx",trackPtErrVtx,"trackPtErrVtx[nVertices][300]/F");
    vertexTree->Branch("trackXVtx",trackXVtx,"trackXVtx[nVertices][300]/F");
    vertexTree->Branch("trackYVtx",trackYVtx,"trackYVtx[nVertices][300]/F");
    vertexTree->Branch("trackZVtx",trackZVtx,"trackZVtx[nVertices][300]/F");
    vertexTree->Branch("trackXYErrVtx",trackYVtx,"trackXYErrVtx[nVertices][300]/F");
    vertexTree->Branch("trackZErrVtx",trackZVtx,"trackZErrVtx[nVertices][300]/F");
    vertexTree->Branch("trackHPVtx",trackHPVtx,"trackHPVtx[nVertices][300]/O");
  }
}

// ------------ method called once each job just after ending the event loop  ------------
void 
IsoVertexIDTreeAnalyzer::endJob() 
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
IsoVertexIDTreeAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(IsoVertexIDTreeAnalyzer);
