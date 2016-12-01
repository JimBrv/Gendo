<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
    $this->load->view('header');
?>

<div class="div-user-change">
	<div class="div-tab-nav">
    	<div class="div-tab-nav-split0"></div>
    	    <a href="change_nick" hidefocus="true"> 
    	         <div class="div-tab-nav-1">修改昵称</div>
    	    </a>
    	    
    	    <div class="div-tab-nav-split1"></div>
    	    <a href="change_email" hidefocus="true"> 
    	         <div class="div-tab-nav-0">修改邮箱</div>
    	    </a>
    
    	    <div class="div-tab-nav-split1"></div>
    	    <a href="change_password" hidefocus="true"> 
    	         <div class="div-tab-nav-1">修改密码</div>
    	    </a>    	   
    	</div>
    	
    	<div class="div-tab-nav-split2"></div>
    	<div style="float:left;height=30px;width:100%;margin-top:30px;margin-left:30px;">
        <?php 
    	    echo "<span style=\"font-size:14px;color:black;margin-top:30px;\">旧Email：".
    	    $this->session->userdata['email'].
    	    "</span>";
    	?> 
    	</div>
    	
    	<div style="float:left;width:100%;margin-top:30px;margin-left:30px;">
    	
        <form action="<?php echo base_url();?>index.php/user/modify_email" method="post">
            <p><span style="font-size:14px;color:blue;diplay:block;">邮箱能找回密码，千万不要随便填哦！</span></p>
            <p><span style="font-size:14px;color:black;">新Email:</span></p>
            
        	<input style="font-size:14px;color:black;" type="text" name="new_email" id="new_email" value="" maxlength="32" /> 
            <span id='msg_nickname' style="color:#4BB2F6">为了最快找回密码，请填入有效的邮箱</span>
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