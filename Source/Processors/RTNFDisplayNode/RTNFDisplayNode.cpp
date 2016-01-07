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

    //Parameter& p =  parameters.getReference(parameterIndex);
    //p.setValue(newValue, 0);

    //threshold = newValue;

    //std::cout << float(p[0]) << std::endl;
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
        

    double sumPower = 0.0;
    //iterate over the channels
    for(int chan_id =0; chan_id< total_channels; ++chan_id){
        current_samples = getNumSamples(chan_id);
        //iterate over samples for each channel and compute sum squared
        double tempChanSumPower = 0.0;
        for(int sample_idx = 0; sample_idx < current_samples; ++sample_idx){
            double sample_value = dataBuffer.getSample(chan_id, sample_idx);
            tempChanSumPower += sample_value*sample_value;
        }
        //store power for each channel power
        current_channel_power[chan_id] = tempChanSumPower;
        
        sumPower += tempChanSumPower;
    }
    globalPower = sumPower/total_channels;
}

