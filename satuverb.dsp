import("stdfaust.lib");
//declare dattorro_rev author "Jakob Zerbian";
//declare dattorro_rev licence "MIT-style STK-4.3 license";
declare latency_samples "7679";

Q = .71;

stereo_wetdry(wet_percent, l1, r1, l2, r2) = l1 * wet_percent + l2 * (1-wet_percent), r1 * wet_percent + r2 * (1-wet_percent);

mod_dattorro_rev(pre_delay, bw, i_diff1, i_diff2, decay, d_diff1, d_diff2, damping, highpass_freq, lowpass_freq, peak_freq, peak_q, peak_gain, sat_drive, sat_wet, sat_postgain) = 
    si.bus(2) : + : *(0.5) : predelay : bw_filter : diffusion_network <: ((si.bus(4) :> renetwork) ~ ro.cross(2))
with {
    // allpass using delay with fixed size
    allpass_f(t, a) = (+ <: @(t),*(a)) ~ *(-a) : mem,_ : +;

    // input pre-delay and diffusion
    predelay = @(pre_delay);
    bw_filter = *(bw) : +~(mem : *(1-bw));
    diffusion_network = allpass_f(142, i_diff1) : allpass_f(107, i_diff1) : allpass_f(379, i_diff2) : allpass_f(277, i_diff2);

    // reverb tail effects
    do_highpass = fi.resonhp(highpass_freq, Q, 1);
    do_lowpass = fi.resonlp(lowpass_freq, Q, 1);
    do_bell = fi.peak_eq_cq(peak_gain,peak_freq,peak_q);
    saturate = _ <: _ * (1-sat_wet) + (_ * sat_drive : aa.clip(-1,1) : aa.parabolic2 : *(sat_wet) : *(sat_postgain));

    // reverb loop
    renetwork = par(i, 2, block(i)) with {
        d = (672, 908, 4453, 4217, 1800, 2656, 3720, 3163);
        block(i) = allpass_f(ba.take(i+1, d),-d_diff1) : @(ba.take(i+3, d)) : damp : 
            saturate : do_highpass : do_lowpass : do_bell : 
            allpass_f(ba.take(i+5, d), d_diff2) : @(ba.take(i+5, d)) : *(decay)
        with {
            damp = *(1-damping) : +~*(damping) : *(decay);
        };
    };
};

// Reverb parameters
bandwidth = 1;//we want to shape this ourselves 
df_1 = .75;
df_2 = .63;
decay_df_1 = .70; 
decay_df_2 = .50; 
damping = 0; //no damping, we want to do our own shaping

// EQ sliders
highpass_freq = vslider("hpfreq[unit:Hz]", 20, 20, 20000, 1);
lowpass_freq = vslider("lpfreq[unit:Hz]", 20000, 20, 20000, 1);
peak_freq = vslider("peakfreq[unit:Hz]", 1000, 20, 20000, 1);
peak_q = vslider("peakq", 50, 0, 100, 1) / 100;
peak_gain = vslider("peakgain[unit:dB]", 0, -20, 20, .1);

// Saturator sliders
sat_drive = vslider("drive[unit:dB]", 0, 0, 40, .1) : ba.db2linear;
sat_wet = vslider("satwet", 100, 0, 100, 1) / 100;
sat_postgain = vslider("postgain[unit:dB]", 0, -40, 0, .1) : ba.db2linear;

// Reverb sliders
effect_wet = hslider("reverbwet", 30, 0, 100, 1) / 100;
decay = hslider("decay", 50, 0, 100, 1) / 100;
pre_delay = hslider("predelay[unit:ms]", 0, 0, 1000, 1) / 1000 * ma.SR;

process = _,_ <: mod_dattorro_rev(pre_delay, bandwidth, df_1, df_2, decay, decay_df_1, decay_df_2, damping,
        highpass_freq, lowpass_freq, peak_freq, peak_q, peak_gain,
        sat_drive, sat_wet, sat_postgain), (_,_ : @(7679),@(7679)) :
        stereo_wetdry(effect_wet);
