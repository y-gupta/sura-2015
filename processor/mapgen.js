var gm = require('gm')
  , resizeX = 343
  , resizeY = 257

gm('base.png')
.blur(30, 20)
.autoOrient()
.write('out.png', function (err) {
  if (err) console.log(err);
  //console.log(response);
});