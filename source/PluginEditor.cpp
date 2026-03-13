#include "PluginEditor.h"

namespace
{
    void drawPanel(juce::Graphics& g, juce::Rectangle<float> area, const juce::String& title)
    {
        g.setColour(juce::Colour(0xff10161d));
        g.fillRoundedRectangle(area, 18.0f);
        g.setColour(juce::Colour(0x44d8f8ff));
        g.drawRoundedRectangle(area, 18.0f, 1.0f);

        const auto titleBounds = juce::Rectangle<int>(
            juce::roundToInt(area.getX()) + 14,
            juce::roundToInt(area.getY()) + 8,
            juce::roundToInt(area.getWidth()) - 28,
            22);

        g.setColour(juce::Colours::white.withAlpha(0.82f));
        g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
        g.drawText(title, titleBounds, juce::Justification::centredLeft, false);
    }

    void drawMeter(juce::Graphics& g, juce::Rectangle<float> area, float value, const juce::String& label, juce::Colour colour)
    {
        g.setColour(juce::Colour(0xff12181f));
        g.fillRoundedRectangle(area, 10.0f);
        g.setColour(juce::Colours::white.withAlpha(0.12f));
        g.drawRoundedRectangle(area, 10.0f, 1.0f);

        const auto inner = area.reduced(4.0f);
        const float height = inner.getHeight() * juce::jlimit(0.0f, 1.0f, value);
        const juce::Rectangle<float> fill(inner.getX(), inner.getBottom() - height, inner.getWidth(), height);

        g.setGradientFill(juce::ColourGradient(colour.brighter(), fill.getCentreX(), fill.getBottom(), colour.darker(0.8f), fill.getCentreX(), fill.getY(), false));
        g.fillRoundedRectangle(fill, 7.0f);

        g.setColour(juce::Colours::white.withAlpha(0.72f));
        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        const auto labelBounds = juce::Rectangle<int>(area.toNearestInt().getX() - 4, area.toNearestInt().getBottom() + 6, area.toNearestInt().getWidth() + 8, 14);
        g.drawText(label, labelBounds, juce::Justification::centred, false);
    }

    void layoutKnobGrid(juce::Rectangle<int> area, std::initializer_list<LabeledKnob*> knobs, int columns)
    {
        if (knobs.size() == 0 || columns <= 0)
            return;

        constexpr int gap = 12;
        const int count = (int) knobs.size();
        const int rows = (count + columns - 1) / columns;
        const int cellW = (area.getWidth() - (columns - 1) * gap) / columns;
        const int cellH = (area.getHeight() - (rows - 1) * gap) / rows;

        int index = 0;
        for (auto* knob : knobs)
        {
            const int row = index / columns;
            const int col = index % columns;
            knob->setBounds(area.getX() + col * (cellW + gap), area.getY() + row * (cellH + gap), cellW, cellH);
            ++index;
        }
    }
}

LabeledKnob::LabeledKnob()
{
    addAndMakeVisible(slider);
    addAndMakeVisible(title);
    addAndMakeVisible(footer);

    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffd8f8ff));
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0x334f6f7f));
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff7ce6ff));
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff151a21));

    for (auto* label : { &title, &footer })
    {
        label->setJustificationType(juce::Justification::centred);
        label->setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.88f));
    }

    title.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    footer.setFont(juce::FontOptions(10.0f, juce::Font::plain));
}

void LabeledKnob::resized()
{
    auto bounds = getLocalBounds();
    title.setBounds(bounds.removeFromTop(16));
    footer.setBounds(bounds.removeFromBottom(14));
    slider.setBounds(bounds.reduced(2));
}

void ScopeComponent::paint(juce::Graphics& g)
{
    const auto area = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff0d1218));
    g.fillRoundedRectangle(area, 14.0f);
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawRoundedRectangle(area, 14.0f, 1.0f);

    juce::Path path;
    const float pad = 10.0f;
    const float width = area.getWidth() - pad * 2.0f;
    const float height = area.getHeight() - pad * 2.0f;
    const float centreY = area.getCentreY();

    path.startNewSubPath(area.getX() + pad, centreY);
    for (size_t i = 0; i < scopeData.size(); ++i)
    {
        const float x = area.getX() + pad + ((float) i / (float) (scopeData.size() - 1)) * width;
        const float y = centreY - scopeData[i] * height * 0.42f;
        path.lineTo(x, y);
    }

    g.setColour(juce::Colour(0xff9feeff).withAlpha(0.15f));
    g.strokePath(path, juce::PathStrokeType(5.0f));
    g.setColour(juce::Colour(0xffd8f8ff));
    g.strokePath(path, juce::PathStrokeType(1.5f));
}

void ScopeComponent::timerCallback()
{
    processor.copyScopeData(scopeData);
    repaint();
}

void SpectrumComponent::paint(juce::Graphics& g)
{
    const auto area = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff0d1218));
    g.fillRoundedRectangle(area, 14.0f);
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawRoundedRectangle(area, 14.0f, 1.0f);

    const auto inner = area.reduced(10.0f);
    const float barW = inner.getWidth() / (float) bins.size();

    for (size_t i = 0; i < bins.size(); ++i)
    {
        const float h = bins[i] * inner.getHeight();
        juce::Rectangle<float> bar(inner.getX() + barW * (float) i,
                                   inner.getBottom() - h,
                                   juce::jmax(1.0f, barW - 1.0f),
                                   h);
        const auto colour = juce::Colour::fromHSV(0.56f - 0.18f * bins[i], 0.58f, 0.94f, 0.85f);
        g.setColour(colour);
        g.fillRect(bar);
    }
}

void SpectrumComponent::timerCallback()
{
    processor.copyAnalyzerData(bins);
    repaint();
}

BadlineDnBAudioProcessorEditor::BadlineDnBAudioProcessorEditor(BadlineDnBAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      scope(p),
      spectrum(p)
{
    setSize(1520, 960);

    for (auto* button : { &oscBtn, &motionBtn, &toneBtn, &performBtn, &globalBtn })
    {
        styleTab(*button);
        addAndMakeVisible(*button);
    }

    oscBtn.onClick = [this] { switchPage(Page::Osc); };
    motionBtn.onClick = [this] { switchPage(Page::Motion); };
    toneBtn.onClick = [this] { switchPage(Page::Tone); };
    performBtn.onClick = [this] { switchPage(Page::Perform); };
    globalBtn.onClick = [this] { switchPage(Page::Global); };

    for (auto* page : { &oscPage, &motionPage, &tonePage, &performPage, &globalPage })
        addAndMakeVisible(*page);

    styleHeader(oscPageHeader, "OSCILLATORS, REESE STACKS & SUB");
    styleHeader(motionPageHeader, "MACROS, MODULATION & CHARACTER");
    styleHeader(tonePageHeader, "FILTERS, DISTORTION & TONE");
    styleHeader(performPageHeader, "GLIDE, ENVELOPES & NOTE MODES");
    styleHeader(globalPageHeader, "SPACE, HARMONY & MASTER");

    oscPage.addAndMakeVisible(oscPageHeader);
    motionPage.addAndMakeVisible(motionPageHeader);
    tonePage.addAndMakeVisible(tonePageHeader);
    performPage.addAndMakeVisible(performPageHeader);
    globalPage.addAndMakeVisible(globalPageHeader);

    oscPage.addAndMakeVisible(scope);
    tonePage.addAndMakeVisible(spectrum);

    addKnobToPage(oscPage, oscAWave, "OSC A WAVE", "Main table", "oscA_wave");
    addKnobToPage(oscPage, oscAWarp, "OSC A WARP", "Shape push", "oscA_warp");
    addKnobToPage(oscPage, oscALevel, "OSC A LVL", "Front weight", "oscA_level");
    addKnobToPage(oscPage, oscBWave, "OSC B WAVE", "Counter layer", "oscB_wave");
    addKnobToPage(oscPage, oscBWarp, "OSC B WARP", "Stack grit", "oscB_warp");
    addKnobToPage(oscPage, oscBLevel, "OSC B LVL", "Pair volume", "oscB_level");
    addKnobToPage(oscPage, oscCWave, "OSC C WAVE", "Mid body", "oscC_wave");
    addKnobToPage(oscPage, oscCWarp, "OSC C WARP", "Aux grind", "oscC_warp");
    addKnobToPage(oscPage, oscCLevel, "OSC C LVL", "Support", "oscC_level");
    addKnobToPage(oscPage, subLevel, "SUB LEVEL", "Mono low end", "sub_level");
    addKnobToPage(oscPage, subDrive, "SUB DRIVE", "Sine crush", "sub_drive");
    addKnobToPage(oscPage, subOctave, "SUB OCT", "Extra weight", "sub_octave");
    addKnobToPage(oscPage, noiseLevel, "NOISE", "Air & fizz", "noise_level");
    addKnobToPage(oscPage, reeseDetune, "REESE DTN", "Jump-up stack", "reese_detune");
    addKnobToPage(oscPage, reeseWidth, "REESE WID", "Stereo spread", "reese_width");
    addKnobToPage(oscPage, reeseDrift, "REESE DRFT", "Loose motion", "reese_drift");

    addKnobToPage(motionPage, macro1, "MACRO 1", "Motion", "macro1");
    addKnobToPage(motionPage, macro2, "MACRO 2", "Growl", "macro2");
    addKnobToPage(motionPage, macro3, "MACRO 3", "Width/Talk", "macro3");
    addKnobToPage(motionPage, macro4, "MACRO 4", "Drop", "macro4");
    addKnobToPage(motionPage, lfoRate, "LFO RATE", "Swing speed", "lfo1_rate");
    addKnobToPage(motionPage, lfoAmt, "LFO AMT", "Wobble depth", "lfo1_amt");
    addKnobToPage(motionPage, envAmt, "ENV AMT", "Pluck amount", "envamt");
    addKnobToPage(motionPage, buzzAmt, "BUZZ", "Virji rasp", "buzz_amt");
    addKnobToPage(motionPage, talkAmt, "TALK", "Vocal lane", "talk_amt");
    addKnobToPage(motionPage, biteAmt, "BITE", "Attack teeth", "bite_amt");
    addKnobToPage(motionPage, punchAmt, "PUNCH", "Knock", "punch_amt");
    addKnobToPage(motionPage, wobbleAmt, "WOBBLE", "LFO swing", "wobble_amt");
    addKnobToPage(motionPage, hornBend, "HORN BEND", "Warp horn", "horn_bend");
    addKnobToPage(motionPage, hornFormant, "FORMANT", "Mouth size", "horn_formant");
    addKnobToPage(motionPage, hornBody, "HORN BODY", "Chest tone", "horn_body");
    addKnobToPage(motionPage, screechDrive, "SCREECH DRV", "Metal edge", "screech_drive");
    addKnobToPage(motionPage, screechFm, "SCREECH FM", "Tear factor", "screech_fm");
    addKnobToPage(motionPage, fmGrit, "FM GRIT", "Wreck the phase", "fm_grit");

    addKnobToPage(tonePage, cutoff, "FILTER 1 CUT", "Main move", "filter_cutoff");
    addKnobToPage(tonePage, res, "FILTER 1 RES", "Nasal peak", "filter_res");
    addKnobToPage(tonePage, filterDrive, "FILTER DRV", "Pre heat", "filter_drive");
    addKnobToPage(tonePage, filter2Cutoff, "FILTER 2 CUT", "Formant band", "filter2_cutoff");
    addKnobToPage(tonePage, filter2Res, "FILTER 2 RES", "Talk peak", "filter2_res");
    addKnobToPage(tonePage, notchAmt, "NOTCH", "Hollow cut", "notch_amt");
    addKnobToPage(tonePage, distDrive, "DIST DRIVE", "Main aggression", "dist_drive");
    addKnobToPage(tonePage, distMix, "DIST MIX", "Parallel dirt", "dist_mix");
    addKnobToPage(tonePage, outputClip, "OUT CLIP", "Master shave", "output_clip");

    addKnobToPage(globalPage, harmMix, "HARM MIX", "Octaves/5ths", "harm_mix");
    addKnobToPage(globalPage, harmInterval, "HARM INT", "Semitone jump", "harm_interval");
    addKnobToPage(globalPage, harmSpread, "HARM WIDE", "Layer split", "harm_spread");
    addKnobToPage(globalPage, airAmt, "AIR", "Top sheen", "air_amt");
    addKnobToPage(globalPage, stereoSpin, "STEREO", "Side energy", "stereo_spin");
    addKnobToPage(globalPage, monoBlend, "MONO BLEND", "Club focus", "mono_blend");
    addKnobToPage(globalPage, masterGain, "MASTER", "Final output", "master_gain");

    addKnobToPage(performPage, glide, "GLIDE", "Portamento", "glide_ms");
    addKnobToPage(performPage, attack, "ATTACK", "Transient rise", "envA");
    addKnobToPage(performPage, decay, "DECAY", "Bass fall", "envD");
    addKnobToPage(performPage, sustain, "SUSTAIN", "Hold body", "envS");
    addKnobToPage(performPage, release, "RELEASE", "Tail time", "envR");

    styleHeader(playModeLabel, "PLAY MODE");
    styleHeader(notePriorityLabel, "VOICE STEAL");
    performPage.addAndMakeVisible(playModeLabel);
    performPage.addAndMakeVisible(notePriorityLabel);

    styleCombo(playMode);
    styleCombo(notePriority);
    performPage.addAndMakeVisible(playMode);
    performPage.addAndMakeVisible(notePriority);

    playMode.addItem("Mono", 1);
    playMode.addItem("Legato", 2);
    playMode.addItem("Poly", 3);

    notePriority.addItem("Last", 1);
    notePriority.addItem("Low", 2);
    notePriority.addItem("High", 3);

    playModeAtt = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "play_mode", playMode);
    notePriorityAtt = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "note_priority", notePriority);

    switchPage(Page::Osc);
    startTimerHz(20);
}

void BadlineDnBAudioProcessorEditor::styleTab(juce::TextButton& button)
{
    button.setClickingTogglesState(true);
    button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff11161d));
    button.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff233244));
    button.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.78f));
    button.setColour(juce::TextButton::textColourOnId, juce::Colour(0xffd8f8ff));
}

void BadlineDnBAudioProcessorEditor::styleCombo(juce::ComboBox& combo)
{
    combo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff11161d));
    combo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0x44d8f8ff));
    combo.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    combo.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xffd8f8ff));
}

void BadlineDnBAudioProcessorEditor::styleHeader(juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.88f));
    label.setFont(juce::FontOptions(15.0f, juce::Font::bold));
}

void BadlineDnBAudioProcessorEditor::setupKnob(LabeledKnob& knob, const juce::String& header, const juce::String& footer)
{
    knob.title.setText(header, juce::dontSendNotification);
    knob.footer.setText(footer, juce::dontSendNotification);
}

void BadlineDnBAudioProcessorEditor::addKnobToPage(juce::Component& page,
                                                   LabeledKnob& knob,
                                                   const juce::String& title,
                                                   const juce::String& footer,
                                                   const juce::String& paramID)
{
    setupKnob(knob, title, footer);
    page.addAndMakeVisible(knob);
    attach(knob, paramID);
}

void BadlineDnBAudioProcessorEditor::attach(LabeledKnob& knob, const juce::String& paramID)
{
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, paramID, knob.slider));
}

juce::Component& BadlineDnBAudioProcessorEditor::pageComponent(Page p)
{
    switch (p)
    {
        case Page::Osc: return oscPage;
        case Page::Motion: return motionPage;
        case Page::Tone: return tonePage;
        case Page::Perform: return performPage;
        default: return globalPage;
    }
}

juce::String BadlineDnBAudioProcessorEditor::pageTitle(Page p) const
{
    switch (p)
    {
        case Page::Osc: return "OSC / REESE / SUB";
        case Page::Motion: return "MACROS / WOBBLE / CHARACTER";
        case Page::Tone: return "FILTER / DISTORT / ANALYZER";
        case Page::Perform: return "PLAY MODES / GLIDE / ENVELOPE";
        default: return "SPACE / HARMONY / MASTER";
    }
}

void BadlineDnBAudioProcessorEditor::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.fillAll(juce::Colour(0xff080b10));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff0b1016), 0.0f, 0.0f, juce::Colour(0xff171d27), 0.0f, bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds.reduced(10.0f), 28.0f);
    g.setColour(juce::Colour(0x33d8f8ff));
    g.drawRoundedRectangle(bounds.reduced(10.0f), 28.0f, 1.2f);

    g.setColour(juce::Colours::white.withAlpha(0.98f));
    g.setFont(juce::FontOptions(35.0f, juce::Font::bold));
    g.drawText("Blackside Bass", 26, 16, 380, 36, juce::Justification::centredLeft, false);

    g.setColour(juce::Colours::white.withAlpha(0.44f));
    g.setFont(juce::FontOptions(14.0f, juce::Font::plain));
    g.drawText("L1M10 patch :: jump-up, bassline and screw-face control set", 30, 52, 560, 20, juce::Justification::centredLeft, false);

    drawPanel(g, pageComponent(currentPage).getBounds().toFloat(), pageTitle(currentPage));
    drawMeter(g, { 1436.0f, 34.0f, 20.0f, 86.0f }, audioProcessor.getMeterLevel(), "OUT", juce::Colour(0xffd8f8ff));
    drawMeter(g, { 1464.0f, 34.0f, 20.0f, 86.0f }, audioProcessor.getSubMeterLevel(), "SUB", juce::Colour(0xff7ce6ff));
}

void BadlineDnBAudioProcessorEditor::resized()
{
    constexpr int topY = 88;
    constexpr int navH = 34;
    constexpr int navW = 110;

    oscBtn.setBounds(26, topY, navW, navH);
    motionBtn.setBounds(144, topY, navW, navH);
    toneBtn.setBounds(262, topY, navW, navH);
    performBtn.setBounds(380, topY, navW, navH);
    globalBtn.setBounds(498, topY, navW, navH);

    auto content = getLocalBounds().reduced(24);
    content.removeFromTop(132);

    for (auto* page : { &oscPage, &motionPage, &tonePage, &performPage, &globalPage })
        page->setBounds(content);

    auto oscArea = oscPage.getLocalBounds().reduced(18);
    oscPageHeader.setBounds(oscArea.removeFromTop(24));
    oscArea.removeFromTop(8);
    auto oscTop = oscArea.removeFromTop(276);
    scope.setBounds(oscTop.removeFromLeft(540).reduced(6));
    layoutKnobGrid(oscTop, { &oscAWave, &oscAWarp, &oscALevel, &oscBWave, &oscBWarp, &oscBLevel, &oscCWave, &oscCWarp }, 4);
    oscArea.removeFromTop(12);
    layoutKnobGrid(oscArea.removeFromTop(250), { &oscCLevel, &subLevel, &subDrive, &subOctave, &noiseLevel, &reeseDetune, &reeseWidth, &reeseDrift }, 4);

    auto motionArea = motionPage.getLocalBounds().reduced(18);
    motionPageHeader.setBounds(motionArea.removeFromTop(24));
    motionArea.removeFromTop(8);
    layoutKnobGrid(motionArea.removeFromTop(540), { &macro1, &macro2, &macro3, &macro4, &lfoRate, &lfoAmt, &envAmt, &buzzAmt, &talkAmt, &biteAmt, &punchAmt, &wobbleAmt, &hornBend, &hornFormant, &hornBody, &screechDrive, &screechFm, &fmGrit }, 6);

    auto toneArea = tonePage.getLocalBounds().reduced(18);
    tonePageHeader.setBounds(toneArea.removeFromTop(24));
    toneArea.removeFromTop(8);
    auto toneTop = toneArea.removeFromTop(300);
    spectrum.setBounds(toneTop.removeFromLeft(520).reduced(6));
    layoutKnobGrid(toneTop, { &cutoff, &res, &filterDrive, &filter2Cutoff, &filter2Res, &notchAmt, &distDrive, &distMix, &outputClip }, 3);

    auto performArea = performPage.getLocalBounds().reduced(18);
    performPageHeader.setBounds(performArea.removeFromTop(24));
    performArea.removeFromTop(8);
    auto comboRow = performArea.removeFromTop(34);
    playModeLabel.setBounds(comboRow.removeFromLeft(130));
    playMode.setBounds(comboRow.removeFromLeft(160));
    comboRow.removeFromLeft(28);
    notePriorityLabel.setBounds(comboRow.removeFromLeft(130));
    notePriority.setBounds(comboRow.removeFromLeft(160));
    performArea.removeFromTop(16);
    layoutKnobGrid(performArea.removeFromTop(180), { &glide, &attack, &decay, &sustain, &release }, 5);

    auto globalArea = globalPage.getLocalBounds().reduced(18);
    globalPageHeader.setBounds(globalArea.removeFromTop(24));
    globalArea.removeFromTop(8);
    layoutKnobGrid(globalArea.removeFromTop(280), { &harmMix, &harmInterval, &harmSpread, &airAmt, &stereoSpin, &monoBlend, &masterGain }, 4);
}

void BadlineDnBAudioProcessorEditor::switchPage(Page newPage)
{
    currentPage = newPage;

    oscPage.setVisible(newPage == Page::Osc);
    motionPage.setVisible(newPage == Page::Motion);
    tonePage.setVisible(newPage == Page::Tone);
    performPage.setVisible(newPage == Page::Perform);
    globalPage.setVisible(newPage == Page::Global);

    oscBtn.setToggleState(newPage == Page::Osc, juce::dontSendNotification);
    motionBtn.setToggleState(newPage == Page::Motion, juce::dontSendNotification);
    toneBtn.setToggleState(newPage == Page::Tone, juce::dontSendNotification);
    performBtn.setToggleState(newPage == Page::Perform, juce::dontSendNotification);
    globalBtn.setToggleState(newPage == Page::Global, juce::dontSendNotification);

    repaint();
}

void BadlineDnBAudioProcessorEditor::timerCallback()
{
    repaint();
}
