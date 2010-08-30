#define snoise(x) 2*noise(x)-1

surface test_surface(float Ka=1, Kd=1, Ks=0.25, roughness = 0.1;
                     color specularcolor = 1;) {
    normal Nf = faceforward(normalize(N), I);
    /* normal nNf = transform("object", P); */
    float val = sin(32 * (t*3.141592654)) * sin(64 * (s*3.141592654));
    color diff_comp =
        clamp(0,val, 1) * (color "rgb" (0, 1, 0)) + 
        clamp(0,-val, 1)* (color "rgb" (0, 0, 1)) +
        noise(s,t) * (color "rgb" (0.25, 0, 0));
    Ci = (Ka * ambient() + Kd*diff_comp) + Ks*specular(Nf, -normalize(I), roughness);
    /* Oi = Os + snoise(transform("object", P)); */
    /* Oi = color "rgb" (abs(cos(3.141592654*comp(Ci,0))), abs(sin(3.141592654*comp(Ci,1))), abs(sin(3.141592654*comp(Ci,2)))); */
    Ci *= Oi;
}
