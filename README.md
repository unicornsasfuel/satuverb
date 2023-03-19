# Satuverb
![Satuverb Interface](https://github.com/unicornsasfuel/satuverb/raw/main/satuverb.png "Satuverb Interface")

Satuverb is a flexible reverb that has many creative and natural sounding uses. It has a saturator and EQ controls inside the reverberator, which apply every time the signal is fed back. This causes the effects to grow more extreme as the reverb tail trails off.

Analog reverb units have this behavior by their nature; the components used in the delay lines would naturally introduce saturation and shape the frequency spectrum in increasingly dramatic ways as the reverb tail faded.

To download Satuverb, go to the [Releases](releases) section. If you're not sure where to put a VST3 file to install it, check the directions [here](https://helpcenter.steinberg.de/hc/en-us/articles/115000177084-VST-plug-in-locations-on-Windows).

# Reverberator Design
## Topology
Satuverb's reverberator is based on Jon Dattorro's 1997 design, but with a key difference: Where Dattorro's design outputs the incoming signal immediately after passing through the diffuser and before entering the feedback delay network (FDN), Satuverb routes all signal through the FDN before outputting signal.

## Advantages / Drawbacks
The benefit of this change to Dattorro's design is that the output is far more rich and lush and decorrelated from the original signal, making it suitable for use in a reverb send scenario and improving the subjective quality of the output.

The problem introduced with this change, however, is latency. The initial sound produced from the FDN follows the original signal by 150ms+. The fix for this is to delay the dry signal by the same amount, report the delay to the plugin host, and let the delay compensation of the host sync the dry and wet signal so the lush reverb follows the dry signal immediately. While this makes it perfectly usable in studio settings, it is poorly suited to live performance.
