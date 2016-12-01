<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
    $this->load->view('header');
?>

<div class="div-user-change">
	<div class="div-tab-nav">
    	<div class="div-tab-nav-split0"></div>
    	    <a href="change_nick" hidefocus="false"> 
    	         <div class="div-tab-nav-1">修改昵称</div>
    	    </a>
    	    
    	    <div class="div-tab-nav-split1"></div>
    	    <a href="change_email" hidefocus="true"> 
    	         <div class="div-tab-nav-1">修改邮箱</div>
    	    </a>
    
    	    <div class="div-tab-nav-split1"></div>
    	    <a href="change_password" hidefocus="true"> 
    	         <div class="div-tab-nav-0">修改密码</div>
    	    </a>    	   
    	</div>
    	
    	<div class="div-tab-nav-split2"></div>
    	<div style="float:left;height=30px;width:100%;margin-top:30px;margin-left:30px;">
        <?php 
    	    echo "<span style=\"font-size:14px;color:black;margin-top:30px;\">旧密码：".
    	    "<span style=\"font-size:12px;color:blue;\">已隐形...</span>".
    	    "</span>";
    	?> 
    	</div>
    	
    	<div style="float:left;width:100%;margin-top:30px;margin-left:30px;">
    	<span style="font-size:14px;color:blue;"></span>
        <form action="<?php echo base_url();?>index.php/user/modify_password" method="post">
            <span style="font-size:14px;color:black;">新密码:</span>
        	<input style="font-size:14px;color:black;" type="password" name="new_password" id="new_password" value="" maxlength="12"/> 
        	<p></p>
            <span style="font-size:14px;color:black;">请确认:</span>
        	<input style="font-size:14px;color:black;" type="password" name="new_passconf" id="new_passconf" value="" maxlength="12"/>         	

        	<span id='msg_password' style="color:#4BB2F6">请牢记您的新密码</span>
            <div style="margin-top:30px;">
                 <input class="alert_ok" type="submit" name="submit" value="提交">
            </div>
       </form>
       </div>
</div>

<script type="text/javascript">
    $(document).ready(function() {
        $('#new_passconf').blur(function() { 
            $.ajax({
                    url: '<?php echo base_url().'index.php/register/ajax_check_password'; ?>',
                    type: 'POST',
                    data: {password : $('#new_password').val(), passconf : $('#new_passconf').val()},
                    success: function(response) {                                
                                      $('#msg_password').html(response);
                                      $('#msg_password').fadeIn();
                             }
                    })
        });
        
$(document).ready(function() {
	    $('.div-tab-nav-1').click(function() { 
		$($this).removeClass('div-tab-nav-1');
		$($this).addClass('div-tab-nav-0');
    });  	
});
 	createCode();
</script>
<?php $this->load->view('footer');?>