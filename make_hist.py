#!/usr/bin/env python2
import os
import subprocess
import toolbox
import ROOT
ROOT.gROOT.SetBatch(True)

#----------------------------------------------------------------------------------------------------#
# global variables
#----------------------------------------------------------------------------------------------------#
filename = "fakePhoton_GJet.root"
directory = "/eos/user/y/ykao/www/tprimetH_THQHadronic/test_01/"
directory = "/eos/user/y/ykao/www/tprimetH_THQHadronic/test/"

c1 = ROOT.TCanvas("c1", "", 800, 600)

#----------------------------------------------------------------------------------------------------#
# Auxiliary functions
#----------------------------------------------------------------------------------------------------#
def clear_directory(d):
    if os.path.exists(d):
        subprocess.call("rm -r %s" % d, shell=True)

def create_directory(dir_output):
    php_index = "/eos/user/y/ykao/www/index.php"
    subprocess.call( "mkdir -p %s" % dir_output, shell=True)
    subprocess.call( "cp -p %s %s" % (php_index, dir_output), shell=True)

def update_to_my_webpage(myfigure):
    subprocess.call("cp -p %s %s" % (myfigure, directory), shell = True)

#----------------------------------------------------------------------------------------------------
# retrieve fake photon IDMVA
#----------------------------------------------------------------------------------------------------#
def make_fake_photon_IDMVA(var, output, do_fit = False):
    f1 = ROOT.TFile.Open(filename, "R")
    tree = f1.Get("t")

    hist = ROOT.TH1D("hist", ";Photon ID MVA; Entries", 40, -1, 1)
    tree.Draw("%s >> hist" % var)

    hist.GetXaxis().SetRangeUser(-0.9,1.0)
    if do_fit:
        hist.Fit("pol7")
    hist.Draw("e1")

    c1.SaveAs(output)
    update_to_my_webpage(output)

#----------------------------------------------------------------------------------------------------#
# Execution
#----------------------------------------------------------------------------------------------------#
if __name__ == "__main__":
    clear_directory(directory)
    create_directory(directory)
    make_fake_photon_IDMVA("fake_photon_IDMVA", "check_fake_photon_IDMVA_GammaJets.png", True)
    make_fake_photon_IDMVA("maxIDMVA_", "check_max_photon_IDMVA_GammaJets.png")
    make_fake_photon_IDMVA("minIDMVA_", "check_min_photon_IDMVA_GammaJets.png")
