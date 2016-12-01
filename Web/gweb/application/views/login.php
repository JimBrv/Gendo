
<div id="loginbg">
</div>

<div id="login">
    <input id="close_login" type="button" title="退出登录" value="x" onClick="close_login()" />
    <h2>跟斗云登录</h2>
　　 <form id="login_form" action="<?php echo base_url()?>login" method="POST">
    	<p><label>账户：</label><input type="text" name="username" id="username" class="text"></p>
    	<p><label>密码：</label><input type="password" name="password" id="password" class="text"></p>
    	 <p><input id="remember" name="remember" type="checkbox" style="margin-left:10%;margin-top:5px;"/><label for="remember" style="font-size:12px;"> 下次自动登录</label></p>
    	<p><input type="submit" name="login" id="" value="登 录" class="btn" style="margin-left:40%;" onclick="on_submit();" ></p>
       
    </form>
</div>