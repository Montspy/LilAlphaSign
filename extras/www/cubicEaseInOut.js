NUM_LEDS_TO_EASE = 34

function easeInCubic(t) {
    return Math.pow(t,3);
}

function easeOutCubic(t) {
    return 1 - easeInCubic(1-t);
}

function easeInOutCubic(t) {
    if(t < 0.5) return easeInCubic(t*2.0)/2.0;
    return 1-easeInCubic((1-t)*2)/2;
}

function easeInQuad(t) {  return t*t; }
function easeOutQuad(t) { return 1-easeInQuad(1-t); }

function easeInOutQuad(t) {
    if(t < 0.5) return easeInQuad(t*2.0)/2.0;
    return 1-easeInQuad((1-t)*2)/2;                
}

v = 0
out = ""

for(i = 0; i < NUM_LEDS_TO_EASE; i++) {
    v = 1.0 * i / (NUM_LEDS_TO_EASE - 1)
    //console.log(v + ", " + easeInOutQuad(v) + ", " + Math.round(NUM_LEDS_TO_EASE * easeInOutQuad(v)))
    out += Math.round(NUM_LEDS_TO_EASE * easeInOutQuad(v)) + ", "
}

console.log(out)