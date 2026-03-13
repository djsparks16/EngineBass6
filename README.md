Blackside Bass :: L1M10 rebuild patch

Honest audit of the previous patch:
- The previous zip claimed the float-to-int warning in PluginEditor.cpp was fixed, but the warning-causing float drawText call was still present.
- It claimed page ownership was corrected, but most knobs were still added to the editor root instead of the page containers.
- It claimed macro 3 and macro 4 had audible impact, but they were not wired in the current DSP.
- It claimed mono note retrigger preserved released-note velocity, but the code still retriggered with velocity 1.0f.
- It claimed the sub oscillator bug was fixed, but the current uploaded files still showed sub phase being read directly instead of being advanced by a proper process call.

What this rebuild actually changes:
1) Real bug fixes
- Fixed the float-to-int warning source in the editor drawing code.
- Fixed ADSR sample-rate setup in the synth voice.
- Fixed the silent/broken sub by adding proper sub and sub-octave oscillator processing.
- Fixed mono note-stack velocity recall.
- Improved mono sustain handling so released notes are pruned correctly when sustain is lifted.
- Added age-based voice stealing for poly mode.
- Fixed DC blocking to use a proper one-pole high-pass state pair.

2) DSP expansion
- Added stereo voice rendering instead of mono duplicated to all channels.
- Added bassline-oriented controls: Buzz, Talk, Bite, Punch, Stereo Spin, FM Grit, Wobble, Notch, Air, Mono Blend.
- Added proper macro3 / macro4 influence across width, talk/notch, clipping and drop energy.
- Added stronger reese, wobble and formant behaviour for UK bassline / jump-up style patches.
- Added sub-octave layer and better transient shaping.

3) UI rebuild
- Rebuilt the editor around real page ownership.
- Expanded the UI to expose the new controls.
- Kept the dark premium layout direction inspired by the screenshot without cloning it.
- Grouped controls into clearer pages: Osc, Motion, Tone, Perform, Global.

4) Build/project cleanup
- Added JUCE header generation in CMake.
- Targeted the explicit VST3 build in the GitHub Actions workflow.
- Packaged everything in a normal source/ layout.
