var ruby = require('./ruby'),
    symbols = require('./symbols');

function getHash(rbMod) {
  return ruby.rubyFixnumToJS(rbMod.callMethod(symbols.hash));
}

function ModuleMap() {
  this._buckets = {};
}

ModuleMap.prototype = {
  set: function(rbMod, jsMod) {
    var hash = getHash(rbMod);
    var bucket = this._buckets[hash];
    if (!bucket)
      bucket = this._buckets[hash] = [];
      
    jsMod._rubyMod = rbMod;
    bucket.push(jsMod);
  },
  get: function(rbMod) {
    var bucket = this._buckets[getHash(rbMod)];
    if (!bucket)
      return null;
    
    for (var i = 0; i < bucket.length; i++) {
      var jsMod = bucket[i];
      if (ruby.rubyBoolToJS(rbMod.callMethod(symbols.equal, jsMod._rubyMod)))
        return jsMod;
    }
    
    return null;
  }
};

module.exports = ModuleMap;
