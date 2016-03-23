/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "RTNFDisplayNode.h"

RTNFDisplayNode::RTNFDisplayNode()
    : GenericProcessor("RTNF Viewer") //, threshold(200.0), state(true)

{

    //parameters.add(Parameter("thresh", 0.0, 500.0, 200.0, 0));

    // load in spacial filter here
    spacial_filter_matrix.load("test_pcc_spacial_filter.mat", arma::raw_ascii);

}

RTNFDisplayNode::~RTNFDisplayNode()
{

}

AudioProcessorEditor* RTNFDisplayNode::createEditor()
{

    editor = new RTNFDisplayEditor(this, true);
    return editor;

}



void RTNFDisplayNode::setParameter(int parameterIndex, float newValue)
{
    editor->updateParameterButtons(parameterIndex);
    editor->updateParameterButtons(parameterIndex);
}

void RTNFDisplayNode::process(AudioSampleBuffer& dataBuffer,
                               MidiBuffer& events)
{
    //initalize only once
    if(total_channels == -1){
        total_channels = dataBuffer.getNumChannels();
        current_channel_power.resize(total_channels);
    }

    /* perform spacial filtering here, remember to apply temporal filter(s) like iir then bilateral and/or weiener filter, but note, could be performance cost */

    // keep track of min samples in channel and truncate sample matrix based on such value (bufferes may not be completely filled each time)

    //iterate over the channels (fill in sample matrix while computing powers)
    arma::mat sample_matrix(total_channels, max_samples_per_buffer);


    double sumPower = 0.0;
    int max_samples = max_samples_per_buffer;

    for(int chan_id =0; chan_id< total_channels; ++chan_id){
        current_samples = getNumSamples(chan_id);
        if(current_samples < max_samples)
            max_samples = current_samples;

        //iterate over samples for each channel and compute sum squared
        double tempChanSumPower = 0.0;
        for(int sample_idx = 0; sample_idx < current_samples; ++sample_idx){
            double sample_value = dataBuffer.getSample(chan_id, sample_idx);
            tempChanSumPower += sample_value*sample_value;
            sample_matrix(chan_id,sample_idx) = sample_value;
        }
        //store power for each channel power
        current_channel_power[chan_id] = tempChanSumPower;
        
        sumPower += tempChanSumPower;
    }
    globalPower = sumPower/total_channels;

    //compute parametric roi values
    arma::mat normed_sample_matrix = sample_matrix.submat(0, 0, total_channels, max_samples);

    //should be max_samples by num dims (x,y,z)
    arma::mat parametric_roi_matrix_transposed = (spacial_filter_matrix * normed_sample_matrix).t();
    double tempRoiPower = 0.0;
    for(int sample_idx = 0; sample_idx < parametric_roi_matrix_transposed.n_rows; ++sample_idx){
        double _x = parametric_roi_matrix_transposed(sample_idx, 0);
        double _y = parametric_roi_matrix_transposed(sample_idx, 1);
        double _z = parametric_roi_matrix_transposed(sample_idx, 2);
        double sample_idx_magv = std::sqrt(std::pow(_x ,2) + std::pow(_y ,2) + std::pow(_z ,2));
        tempRoiPower += sample_idx_magv*sample_idx_magv;
    }
    roiPower = tempRoiPower;
}

