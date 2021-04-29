function setContent(rooms,err) {
  const content = document.querySelector('body > div > div.content');
  
  content.innerHTML = err || '';
  if(rooms) {
    if(rooms.length) {
      rooms.forEach(v => {  
        const item = document.createElement('div');
        const itemRoom = document.createElement('div');
        const itemValue = document.createElement('div');
        
        item.className = 'item-block';
        itemRoom.className = 'item-room-block';
        itemValue.className = 'item-value-block';
        itemRoom.innerText = `Комната: ${v.name}`;
        itemValue.innerText = `${v.t} C`;
        item.appendChild(itemRoom);
        item.appendChild(itemValue);
        content.appendChild(item);
      });
    } else {
      content.innerHTML = 'датчиков нет';
    }
  }
}

let counterFail = 0;
function getTempList() {
  const xhr = new XMLHttpRequest();

  xhr.onload = function () {
    if (xhr.status === 200) {
      const res = JSON.parse(xhr.response);
      if(res) {
        setContent(res);
        setTimeout(getTempList,5000);
      } else {
        if(counterFail === 5) {
          counterFail = 0;
          setContent([]);
          setTimeout(getTempList,15000);
        } else {
          setTimeout(getTempList,3000);
          counterFail++;
        }
      }
    }
  };
  xhr.onprogress = function (event) { /* alert(`Loaded ${event.loaded} of ${event.total}`); */ };
  xhr.onerror = function () { setContent([],'Ошибка получения данных') };

  xhr.open('GET', '/list');
  xhr.send();
}

getTempList();

function test() {
  const xhr = new XMLHttpRequest();
  const rnd = Math.floor(Math.random() * 10);
  const rndT = (Math.random() * (45 - 15 + 1) + 15).toFixed(2);

  console.log(`R: ${rnd}`);
  console.log(`T: ${rndT}`);

  xhr.open("POST", '/setTemp', true);
  xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
  
  xhr.onreadystatechange = function () {//Вызывает функцию при смене состояния.
    if (xhr.readyState == XMLHttpRequest.DONE && xhr.status == 200) {
      // Запрос завершён. Здесь можно обрабатывать результат.
      console.log(xhr.response);
    }
  }
  xhr.send(`name=${rnd}&t=${rndT}`);
}