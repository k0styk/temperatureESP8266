const path = require('path');
const express = require('express');
const Database = require("@replit/database");
const db = new Database();

const port = 3000;
const app = express();

app.use(express.json());
app.use(express.urlencoded({ extended: true }));
app.use(express.static('public'));
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname,'public','index.html'))
});

app.get('/list', async (req,res) => {
  const rooms = [];
  const matches = await db.list('0x');
  
  if(matches) {
    for(let i=0;i<matches.length;i++) {
      const value = await db.get(matches[i]);
      rooms.push({t: value,name:i+1});
    };
  }
  res.json(rooms);
});

app.post('/setTemp', (req,res) => {
  // console.log(req.body);
  // console.log(req.body.name);
  // console.log(req.body.t);

  const deviceAddress = req.body.name;
  const roomT = req.body.t;

  db.set(deviceAddress, roomT);
});

app.listen(port, () => {
  console.log(`Example app listening at http://localhost:${port}`);
  db.list().then(keys => { keys.forEach(k => db.delete(k)); });
});