var ruby = require('./ruby'),
    symbols = require('./symbols');

function ModuleMap() {
  this._buckets = {};
}

ModuleMap.prototype = {
  set: function(rbMod, jsMod) {
    var hash = rbMod.hash();
    var bucket = this._buckets[hash];
    if (!bucket)
      bucket = this._buckets[hash] = [];
      
    jsMod._rubyMod = rbMod;
    bucket.push(jsMod);
  },
  get: function(rbMod) {
    var bucket = this._buckets[rbMod.hash()];
    if (!bucket)
      return null;
    
    for (var i = 0; i < bucket.length; i++) {
      var jsMod = bucket[i];
      if (rbMod.equal(jsMod._rubyMod))
        return jsMod;
    }
    
    return null;
  }
};

module.exports = ModuleMap;
