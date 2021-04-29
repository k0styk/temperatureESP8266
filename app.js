const path = require('path');
const express = require('express');
const bodyParser = require('body-parser');
const Database = require("@replit/database");
const db = new Database();

const port = 3000;
const app = express();

app.use(bodyParser.urlencoded({ extended: false }))
app.use(bodyParser.json())
app.use(express.static('public'));
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname,'public','index.html'))
})

app.get('/list', (req,res) => {
  db.get('rooms').then(value => {
    res.json(value || []);
  });
})

app.get('/:id', (req,res) => {
  // console.log('ID:', req.params.id);
  // db.list("prefix").then(matches => {});  
})

app.post('/setTemp', (req,res) => {
  // console.log(req.body);
  // console.log(req.body.name);
  // console.log(req.body.t);
  db.get('rooms').then(value => {
    let rooms = [];
    const room = {
      name: req.body.name,
      t: req.body.t,
    };

    if(value) {
      const index = value.findIndex(v => v.name === room.name);
      if(index === -1) {
        rooms = [...value, room];
      } else {
        rooms = value.map(v => v.name===room.name?room:v);
      }
    } else {
      rooms.push(room);
    }

    db.set('rooms', rooms).then(() => {});
  })
})

app.listen(port, () => {
  console.log(`Example app listening at http://localhost:${port}`);
  db.list().then(keys => { keys.forEach(k => db.delete(k)); });
})