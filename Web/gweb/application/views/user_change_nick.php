<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
    $this->load->view('header');
?>

<div class="div-user-change">
	<div class="div-tab-nav">
    	<div class="div-tab-nav-split0"></div>
    	    <a href="change_nick" hidefocus="true"> 
    	         <div class="div-tab-nav-0">修改昵称</div>
    	    </a>
    	    
    	    <div class="div-tab-nav-split1"></div>
    	    <a href="change_email" hidefocus="true"> 
    	         <div class="div-tab-nav-1">修改邮箱</div>
    	    </a>
    
    	    <div class="div-tab-nav-split1"></div>
    	    <a href="change_password" hidefocus="true"> 
    	         <div class="div-tab-nav-1">修改密码</div>
    	    </a>    	   
    	</div>
    	
    	<div class="div-tab-nav-split2"></div>
    	
    	<div style="float:left;height=30px;width:100%;margin-top:30px;margin-left:30px;">
    	<p><span style="font-size:14px;color:blue;height=25px;">昵称不能用来登录，但能展现您的个性！</span></p>
        <?php 
    	    echo "<p><span style=\"font-size:14px;color:black;margin-top:30px;\">旧昵称：".
    	    $this->session->userdata['name'].
    	    "</span></p>";
    	?> 
    	</div>
    	
    	<div style="float:left;width:100%;margin-top:30px;;margin-left:30px;">
        <form action="<?php echo base_url();?>index.php/user/modify_nick" method="post">
            <span style="font-size:14px;color:black;">新昵称:</span>
        	<input style="font-size:14px;color:black;" type="text" name="new_nickname" id="new_nickname" value="" maxlength="64"/> 
            <span id='msg_nickname' style="color:#4BB2F6">起个拉风的名号</span>
            <div style="margin-top:30px;">
                 <input class="alert_ok" type="submit" name="submit" value="提交">
            </div>
       </form>
       </div>
</div>

<script type="text/javascript">
$(document).ready(function() {
	    $('.div-tab-nav-1').click(function() { 
		$($this).removeClass('div-tab-nav-1');
		$($this).addClass('div-tab-nav-0');
    });  	
});
</script>

<?php $this->load->view('footer');?>