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
    	    
    	<div class="div-tab-nav-split2"></div>
	</div>
</div>
	    
	    

<!--		<form action="<?php echo base_url();?>index.php/user/modify" method="post">
	    <div class="input_item">
			<div class="input_name">用户:</div>
			<?php
			 echo "<input class=\"input_content\" type=\"text\" name=\"reg_username\" id=\"reg_username\"".
			  "value=\"".$this->session->userdata['username']."\" disabled />";
			?>
			<div class="input_tips"> 
			    <p><span id='msg_user_name' style="color:#4BB2F6">3-16字母数字组成</span></p>
			</div>
		</div>

	    <div class="input_item">		
			<div class="input_name">新密码:</div>
			<input type="password"  class="input_content"  name="new_password" id="new_password" value="" />
            <div class="input_tips">   
				 <p><span id='msg_password' style="color:#4BB2F6">6-12位键盘字符</span></p>
			</div>
		</div>
			
	    <div class="input_item">	
	        <div class="input_name">新确认:</div>
		    <input type="password"  class="input_content" name="new_passconf" id="new_passconf" value="" />

		</div>

	    <div class="input_item">		
			<div class="input_name">新邮箱: </div>
           	<input  class="input_content"  type="text" name="new_email" id="new_email" value="" />
          	
		  	<div class="input_tips">   
				 <p><span id='msg_email' style="color:#4BB2F6">用于找回密码</span></p>
			</div>
        </div>

	    <div class="input_item">		
			<div class="input_name">昵称:</div>
	    	<input  class="input_content" type="text" name="new_nickname" id="new_nickname" value="" />
            <div class="input_tips"> 
            	<p><span id='msg_nickname' style="color:#4BB2F6">起个拉风的名号</span></p>
            </div>
        </div>
        
        <div class="input_item">
        	<div class="input_name">验证码:</div>
        	<div class="varify_area">
        	       <input class="verify_input" type="input_verify" id="inputCode" />
          	       <input class="varify_code" type="text" id="checkCode"  />    
        	</div>
        	<div class="input_tips">
        	    <a href="#" style="font-size:10px;margin-left:10px;" onclick="createCode();">换一个</a>        	    
        	</div>   
        </div>    	 

        
        <div class="input_item">
            <input id="receive" name="receive" type="checkbox" checked="checked" value="1" style="margin-left:10%;"><a href="service_contract" target="_black" style="color:#ff6600;font-size:10px">我已同意服务条款</a>
        </div>        	
         
        <div class="input_item">
         	<input class="register_submit" type="submit" name="submit" value="">
        </div>
        	
        </form>	
	</div>-->

	<script type="text/javascript">
    $(document).ready(function() {
        $('#new_email').blur(function(){
            $.post('<?php echo base_url()?>index.php/register/ajax_check_email', 
                {'email' : $('#new_email').val()}, 
                function(response){
                        $('#msg_email').html(response);
                }
            );
         });
/*  
        $('#reg_username').blur(function() { 
            $.ajax({
                    url: '<?php echo base_url().'index.php/register/ajax_check_user'; ?>',
                    type: 'POST',
                    data: {username : $('#reg_username').val()},
                    //dataType: 'text',
                    success: function(response) {                                
                                      $('#msg_user_name').html(response);
                                      $('#msg_user_name').fadeIn();
                             }
                    });
        });  */
        
        $('#new_passconf').blur(function() { 
            $.ajax({
                    url: '<?php echo base_url().'index.php/register/ajax_check_password'; ?>',
                    type: 'POST',
                    data: {password : $('#new_password').val(), passconf : $('#new_passconf').val()},
                    //dataType: 'text',
                    success: function(response) {                                
                                      $('#msg_password').html(response);
                                      $('#msg_password').fadeIn();
                             }
                    })
        });  
 	});
 	createCode();
    </script>

<?php $this->load->view('footer');?>