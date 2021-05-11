const path = require('path');
const express = require('express');
const Database = require("@replit/database");
const db = new Database();

const port = 3000;
const app = express();

app.use(express.json())
app.use(express.urlencoded({ extended: true }))
app.use(express.static('public'));
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname,'public','index.html'))
})

app.get('/list', async (req,res) => {
  const rooms = [];
  const matches = await db.list('0x');

  if(matches) {
    for(let i=0;i<matches.length;i++) {
      const room = await db.get(matches[i]);
      
      rooms.push({t: room.t,name:i+1});
    };
  }
  res.json(rooms);
});

app.post('/setTemp', (req,res) => {
  // console.log(req.body);
  // console.log(req.body.name);
  // console.log(req.body.t);

  const deviceAddress = req.body.name;
  const room = {
    t: req.body.t,
    date: new Date().getTime()
  }

  db.set(deviceAddress, room);
  res.sendStatus(200);
});

app.listen(port, async () => {
  console.log(`Example app listening at http://localhost:${port}`);
  db.list().then(keys => { keys.forEach(k => db.delete(k)); });
  setInterval(async () => {
    const matches = await db.list('0x');
    const date = new Date().getTime();
  
    if(matches) {
      for(let i=0;i<matches.length;i++) {
        const room = await db.get(matches[i]);
        const diff = (date - room.date)/1000;

        if(diff > 20) {
          await db.delete(matches[i]);
        }
      };
    }
  }, 1000);
});
