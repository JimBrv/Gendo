<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
    $this->load->view('header');
?>

	<div id="register_form">
	    <div class="div_register_title"></div>	
		<label class="register_title">跟斗云用户注册</label>
		<div class="div_register_title"></div>

		<form action="<?php echo base_url();?>index.php/register/submit" method="post">
	    <div class="input_item">
			<div class="input_name">用户名:</div>
			<input class="input_content" type="text" name="reg_username" id="reg_username" value="" />
			<div class="input_tips"> 
			    <p><span id='msg_user_name' style="color:#4BB2F6">3-16字母数字组成</span></p>
			</div>
		</div>

	    <div class="input_item">		
			<div class="input_name">密码:</div>
			<input type="password"  class="input_content"  name="reg_password" id="reg_password" value="" />
            <div class="input_tips">   
				 <p><span id='msg_password' style="color:#4BB2F6">6-12个键盘字符</span></p>
			</div>
		</div>
			
	    <div class="input_item">	
	        <div class="input_name">确认:</div>
		    <input type="password"  class="input_content" name="reg_passconf" id="reg_passconf" value="" />

		</div>

	    <div class="input_item">		
			<div class="input_name">邮箱: </div>
           	<input  class="input_content"  type="text" name="email" id="email" value="" />
          	
		  	<div class="input_tips">   
				 <p><span id='msg_email' style="color:#4BB2F6">用于找回密码</span></p>
			</div>
        </div>

	    <div class="input_item">		
			<div class="input_name">昵称:</div>
	    	<input  class="input_content" type="text" name="nickname" id="nickname" value="" />
            <div class="input_tips"> 
            	<p><span id='msg_nickname' style="color:#4BB2F6">起个拉轰的名字吧！</span></p>
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
	</div>

	<script type="text/javascript">
    $(document).ready(function() {
        $('#email').blur(function(){
            $.post('<?php echo base_url()?>index.php/register/ajax_check_email', 
                {'email' : $('#email').val()}, 
                function(response){
                        $('#msg_email').html(response);
                }
            );
         });
  
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
        });  
        
        $('#reg_passconf').blur(function() { 
            $.ajax({
                    url: '<?php echo base_url().'index.php/register/ajax_check_password'; ?>',
                    type: 'POST',
                    data: {password : $('#reg_password').val(), passconf : $('#reg_passconf').val()},
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
    