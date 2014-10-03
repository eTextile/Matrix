
Rc_min = 2000.0
Rc_max = 15000.0

Vcc = 3.0

for Rp in range(4000, 10000, 100) :
    Vc_min = Rc_min / (Rc_min + Rp) * Vcc
    Vc_max = Rc_max / (Rc_max + Rp) * Vcc
    print Rp, " - Vc_min", "%.2f" % Vc_min,
    print " - Vc_max", "%.2f" % Vc_max,
    print " - Plage ", "%.2f" % (Vc_max - Vc_min),
    print " - Vcc ", "%.2f" % ((Vc_max - Vc_min)/Vcc * 100)
