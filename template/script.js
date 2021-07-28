const pages = Array.from(document.querySelectorAll('#list > img'));
let currentPage = 1;

function changePage(pageNum) {
  const current = pages[pageNum];

  const right = pages[2*pageNum-3];
  const left = pages[2*pageNum-2];

  if (left == null && right == null)return;

  currentPage = pageNum;

  var display_left = document.getElementById('ileft');
  var display_right = document.getElementById('iright');

  if (left != null && right == null)
    display_right.style = 'display: none'
  else{
    display_right.removeAttribute("style")
    display_right.src = right.src;
    display_right.title = 2*pageNum-2
  }
  
  if (left == null)
    display_left.src = pages[0].src;
  else{
    display_left.src = left.src;
  display_left.title = 2*pageNum-1;
  }

  scroll(0,0);

}

function page_jump() {
  var num = Number(document.getElementById('page-number').value);
  if(num % 2 == 0)
    changePage( ( num / 2 ) + 1 );
  else
    changePage( ( num + 1 ) / 2 );
  $(document).ready(function(){$("#jump_modal").modal('hide');})
}

$(document).ready(function(){$("#usage_modal").modal('show');})

changePage(1);

document.getElementById('iright').onclick = event =>changePage(currentPage - 1);
document.getElementById('ileft').onclick = event =>changePage(currentPage + 1);

document.onkeypress = event => {
  switch (event.key.toLowerCase()) {
    // Previous Image
    case 'w':
	   scrollBy(0, -40);
	   break;
    case 'a':
      changePage(currentPage + 1);
      break;
    // Return to previous page
    case 'q':
      window.history.go(-1);
      break;
    // Next Image
    case ' ':
    case 's':
	  scrollBy(0, 40);
      break;
    case 'd':
      changePage(currentPage - 1);
      break;
    case 'p':
      $(document).ready(function(){$("#jump_modal").modal('show');})
      break;
  }// remove arrow cause it won't work
};

document.onkeydown = event =>{
  switch (event.keyCode) {
    case 37: //left
      changePage(currentPage + 1);
      break;
    case 38: //up
      break;
    case 39: //right
      changePage(currentPage - 1);
      break;
    case 40: //down
      break;
  }
};