<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
    $this->load->view('header');
?>
	<div id="register_form">
		<div class="div_register_title"></div>
		<p class="p1">注册失败 </p>
		<p class="p2"> <?php echo "$username"."($nickname)";?> 失败原因：<?php echo "$error";?></p>
		<p class="p2">
		  <?php echo anchor(site_url().'/register', '返回注册页面'); ?>
		</p>
	</div>

<?php $this->load->view('footer');?>	