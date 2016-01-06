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

#ifndef RTNFDISPLAYEDITOR_H_INCLUDED
#define RTNFDISPLAYEDITOR_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"
#include "../../UI/UIComponent.h"
#include "../../UI/DataViewport.h"
#include "../Visualization/DataWindow.h"
#include "RTNFDisplayNode.h"
#include "RTNFDisplayCanvas.h"
#include "../Editors/VisualizerEditor.h"

class Visualizer;

class RTNFDisplayEditor : public VisualizerEditor
{
public:

    RTNFDisplayEditor(GenericProcessor*, bool useDefaultParameterEditors);
    ~RTNFDisplayEditor();

    void buttonCallback(Button* button);

    Visualizer* createNewCanvas();

private:


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RTNFDisplayEditor);

};



#endif  // RTNFDISPLAYEDITOR_H_INCLUDED
