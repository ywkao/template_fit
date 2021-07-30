#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooDataHist.h"
#include "RooGaussian.h"
#include "TCanvas.h"
#include "RooPlot.h"
#include "TTree.h"
#include "TH1D.h"
#include "TRandom.h"
#include "TFile.h"
#include "TString.h"
#include "THStack.h"
#include <vector>
using namespace RooFit;
 
void cosmetic_histogram(TH1D* h, TString xtitle);
void makePad(THStack *hs, TH1D* h);

void template_fit()
{
    // make stack plots {{{
    TString path = "/afs/cern.ch/work/y/ykao/tPrimeExcessHgg/CMSSW_10_6_8/src/tprimetH";
    TString input_file = path + "/shortcut_plots/plots_20210725/myhist_combine_RunII.root";
    TFile *fin = TFile::Open(input_file);
    std::vector<TString> backgrounds = {"QCD_GammaJets_imputed", "DiPhoton", "TTGG", "TTGJets", "TTJets", "VG"};
    std::vector<Int_t> colors = {kOrange+6, kRed+1, kGreen-2, kGreen-7, kSpring+10, kViolet-9};
    std::vector<TH1D*> vH_maxPhotonIDMVA_Bkg;
    std::vector<TH1D*> vH_minPhotonIDMVA_Bkg;

    TH1D *h_maxPhotonIDMVA_Data = (TH1D*) fin->Get("hPhotonMaxIDMVA_fine_Data");
    TH1D *h_minPhotonIDMVA_Data = (TH1D*) fin->Get("hPhotonMinIDMVA_fine_Data");
    cosmetic_histogram(h_maxPhotonIDMVA_Data, "Max Photon ID MVA");
    cosmetic_histogram(h_minPhotonIDMVA_Data, "Min Photon ID MVA");

    TH1D *h_maxPhotonIDMVA_subtracted = new TH1D(*h_maxPhotonIDMVA_Data);
    TH1D *h_minPhotonIDMVA_subtracted = new TH1D(*h_minPhotonIDMVA_Data);

    THStack *hs_max = new THStack("hs_max", "");
    THStack *hs_min = new THStack("hs_min", "");
    for(int i=backgrounds.size()-1; i>=0; --i)
    {
        TH1D *h_max = (TH1D*) fin->Get("hPhotonMaxIDMVA_fine_" + backgrounds[i]);
        TH1D *h_min = (TH1D*) fin->Get("hPhotonMinIDMVA_fine_" + backgrounds[i]);
        h_max -> SetFillColor(colors[i]);
        h_min -> SetFillColor(colors[i]);
        vH_maxPhotonIDMVA_Bkg.push_back(h_max);
        vH_minPhotonIDMVA_Bkg.push_back(h_min);
        hs_max -> Add(h_max);
        hs_min -> Add(h_min);

        if(i>1) {
            h_maxPhotonIDMVA_subtracted -> Add(h_max, -1.0);
            h_minPhotonIDMVA_subtracted -> Add(h_min, -1.0);
        } else {
            printf("%s: %f\n", backgrounds[i].Data(), h_max->Integral(0, h_max->GetSize()-1));
            printf("%s: %f\n", backgrounds[i].Data(), h_min->Integral(0, h_min->GetSize()-1));
        }
    }

    TCanvas *c1 = new TCanvas("c1", "", 1600, 600);
    c1->Divide(2);
    c1->cd(1);
    makePad(hs_max, h_maxPhotonIDMVA_Data);
    c1->cd(2);
    makePad(hs_min, h_minPhotonIDMVA_Data);
    c1->SaveAs("eos_test/photonIDMVA.png");
    //}}}
    // observables, import data{{{
    RooRealVar x("x","Photon ID MVA",-1.,1.);
    RooCategory channel("channel","channel");
    channel.defineType("maxIDMVA",1);
    channel.defineType("minIDMVA",2);
    
    RooDataHist dh_max_data("dh_max_data", "dh_max_data", x, Import(*h_maxPhotonIDMVA_Data));
    RooDataHist dh_min_data("dh_min_data", "dh_min_data", x, Import(*h_minPhotonIDMVA_Data));
    RooDataHist dh_max_subtracted("dh_max_subtracted", "dh_max_subtracted", x, Import(*h_maxPhotonIDMVA_subtracted));
    RooDataHist dh_min_subtracted("dh_min_subtracted", "dh_min_subtracted", x, Import(*h_minPhotonIDMVA_subtracted));
    RooPlot *frame = x.frame(Title("Imported data of photon ID MVA"));
    
    TCanvas *c3 = new TCanvas("c3","c3",1600,600);
    c3->Divide(2);
    c3->cd(1); dh_max_data.plotOn(frame_max); dh_max_subtracted.plotOn(frame_max, MarkerColor(kBlue)); frame_max->Draw();
    c3->cd(2); dh_min_data.plotOn(frame_min); dh_min_subtracted.plotOn(frame_min, MarkerColor(kBlue)); frame_min->Draw();
    c3->SaveAs("eos_test/imported_data.png");
    //}}}
    // build models{{{
    // models for channel #1
    RooDataHist dh_max_qcd("dh_max_qcd", "dh_max_qcd", x, Import(*vH_maxPhotonIDMVA_Bkg[0]));
    RooDataHist dh_max_dip("dh_max_dip", "dh_max_dip", x, Import(*vH_maxPhotonIDMVA_Bkg[1]));
    RooHistPdf pdf_max_qcd("pdf_max_qcd", "pdf_max_qcd", x, dh_max_qcd, 0);
    RooHistPdf pdf_max_dip("pdf_max_dip", "pdf_max_dip", x, dh_max_dip, 0);

    // models for channel #2
    RooDataHist dh_min_qcd("dh_min_qcd", "dh_min_qcd", x, Import(*vH_minPhotonIDMVA_Bkg[0]));
    RooDataHist dh_min_dip("dh_min_dip", "dh_min_dip", x, Import(*vH_minPhotonIDMVA_Bkg[1]));
    RooHistPdf pdf_min_qcd("pdf_min_qcd", "pdf_min_qcd", x, dh_min_qcd, 0);
    RooHistPdf pdf_min_dip("pdf_min_dip", "pdf_min_dip", x, dh_min_dip, 0);

    // key factors to extract
    RooRealVar sf_qcd("sf_qcd","scale_factor",1.0,0.0,5.0);
    RooRealVar sf_dip("sf_dip","scale_factor",1.0,0.0,5.0);

    // norm = luminosity * acceptance * efficiency
    RooRealVar qcd_norm("qcd_norm","qcd norm",118526.56);
    RooRealVar dip_norm("dip_norm","dip norm",41626.71);
    
    RooProduct yield_qcd("ch1_nqcd","ch1 qcd yields", RooArgList(sf_qcd,qcd_norm));
    RooProduct yield_dip("ch1_ndip","ch1 dip yields", RooArgList(sf_dip,dip_norm));

    RooAddPdf ch1_model("ch1_model","decay1 model",
                        RooArgList(pdf_max_qcd,pdf_max_dip),RooArgList(yield_qcd, yield_dip));
    RooAddPdf ch2_model("ch2_model","decay2 model",
                        RooArgList(pdf_min_qcd,pdf_min_dip),RooArgList(yield_qcd, yield_dip));
    //}}}

    /*
    // skip {{{
    // now build the simultaneous model by adding two channels
    RooSimultaneous model("model","model",channel);
    model.addPdf(ch1_model,"maxIDMVA");
    model.addPdf(ch2_model,"minIDMVA");
    
    // joint two data sets, fit together
    RooDataHist data("data","joint data",x,Index(channel),
                    Import("maxIDMVA",dh_max_subtracted),Import("minIDMVA",dh_min_subtracted));
    model.fitTo(data,Minos(true));
    
    TCanvas *c2 = new TCanvas("c2","c2",1600,600);
    c2->Divide(2);

    c2->cd(1); // channel1 only
    RooPlot* frame2 = x.frame();
    //data.plotOn(frame2,Cut("channel==1"));
    dh_max_subtracted.plotOn(frame2);
    model.plotOn(frame2,Slice(channel,"maxIDMVA"),ProjWData(channel,data));
    pdf_max_qcd.plotOn(frame2, LineColor(colors[0]));
    pdf_max_dip.plotOn(frame2, LineColor(colors[1]));
    frame2->Draw();

    c2->cd(2); // chaneel2 only
    RooPlot* frame3 = y.frame();
    //data.plotOn(frame3,Cut("channel==2"));
    dh_min_subtracted.plotOn(frame3);
    model.plotOn(frame3,Slice(channel,"minIDMVA"),ProjWData(channel,data));
    pdf_min_qcd.plotOn(frame3, LineColor(colors[0]));
    pdf_min_dip.plotOn(frame3, LineColor(colors[1]));
    frame3->Draw();

    //c2->cd(3); // sum of the two channels
    //RooPlot* frame1 = x.frame();
    //data.plotOn(frame1);
    //model.plotOn(frame1,ProjWData(channel,data));
    //frame1->Draw();
    
    c2->SaveAs("eos_test/test.png");
    //}}}
    // observables, import data{{{
    RooRealVar x("x","Max photon ID MVA",-1.,1.);
    RooRealVar y("y","Min photon ID MVA",-1.,1.);
    RooCategory channel("channel","channel");
    channel.defineType("maxIDMVA",1);
    channel.defineType("minIDMVA",2);
    
    RooDataHist dh_max_data("dh_max_data", "dh_max_data", x, Import(*h_maxPhotonIDMVA_Data));
    RooDataHist dh_min_data("dh_min_data", "dh_min_data", y, Import(*h_minPhotonIDMVA_Data));
    RooDataHist dh_max_subtracted("dh_max_subtracted", "dh_max_subtracted", x, Import(*h_maxPhotonIDMVA_subtracted));
    RooDataHist dh_min_subtracted("dh_min_subtracted", "dh_min_subtracted", y, Import(*h_minPhotonIDMVA_subtracted));
    RooPlot *frame_max = x.frame(Title("Imported data of max photon ID MVA"));
    RooPlot *frame_min = y.frame(Title("Imported data of min photon ID MVA"));
    
    TCanvas *c3 = new TCanvas("c3","c3",1600,600);
    c3->Divide(2);
    c3->cd(1); dh_max_data.plotOn(frame_max); dh_max_subtracted.plotOn(frame_max, MarkerColor(kBlue)); frame_max->Draw();
    c3->cd(2); dh_min_data.plotOn(frame_min); dh_min_subtracted.plotOn(frame_min, MarkerColor(kBlue)); frame_min->Draw();
    c3->SaveAs("eos_test/imported_data.png");
    //}}}
    */
}

void cosmetic_histogram(TH1D* h, TString xtitle)
{
    h -> SetStats(0);
    h -> SetMarkerStyle(20);
    h -> SetMinimum(0);
    h -> GetXaxis() -> SetTitle(xtitle.Data());
    //h -> SetMaximum(1e+5);
    //h -> SetMinimum(1e-2);
}

void makePad(THStack *hs, TH1D* h)
{
    //gPad->SetLogy(1);
    gPad->SetGrid();
    h->Draw("ep1");
    hs->Draw("hist, same");
    h->Draw("same, ep1");
}
