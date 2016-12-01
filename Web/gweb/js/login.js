

function open_login()
{
    document.getElementById('loginbg').style.display='block';
    document.getElementById('login').style.display='block';
    showloginbg();
}
function close_login()
{
    document.getElementById('loginbg').style.display='none';
    document.getElementById('login').style.display='none';
}
function showloginbg()
{
    var sWidth,sHeight;
    sWidth = screen.width;
    sWidth = document.body.offsetWidth;
    sHeight=document.body.offsetHeight;
    if (sHeight<screen.height){sHeight=screen.height;}
    document.getElementById("loginbg").style.width = sWidth + "px";
    document.getElementById("loginbg").style.height = sHeight + "px";
    document.getElementById("loginbg").style.display = "block";
    document.getElementById("loginbg").style.display = "block";
    document.getElementById("loginbg").style.right = document.getElementById("login").offsetLeft + "px";
}

function on_submit()
{
    /*
     $.ajax({
        url: '<?php echo base_url().'index.php/login'; ?>',
        type: 'POST',
        data: {username : $('#username').val(), password:$('#password').val()},
        success: function(response) {
            if ($response != undefined) {
                $('#msg_error').html(response);
                $('#msg_error').fadeIn();
            }else{
                close_login();
            }
        }
    });*/
};
