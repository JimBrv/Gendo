<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
    $this->load->view('header');
?>

<div class="main_inner" >
    <div class="svc-order">
        <div class="div-sub-header">
	        <div class="div-title20">
	             <strong style="color:#878787"> 订单确认 </strong>
	        </div>
        </div>
        <div style="width:100%">
            <p style="line-height:20px;font-size:12px;color:green;"><strong>请您仔细核对订单，点击“下一步”即可通过支付宝或网银进行支付。</strong></p> 
            <p style="line-height:20px;font-size:12px">如果支付过程<span style="color:red">遇到问题</span>，请联系QQ在线客服：<a href=""><img src="<?php echo base_url();?>css/images/qq-support.jpg" alt="icon" width="48" height="24"></a> </p> 
        </div>
        
        <div style="float:left;width:60%;height:200px;">
        <?php
            echo "<p class=\"p5\" >用户账号：".$username."</p>";
            echo "<p class=\"p5\">订购产品：".$svc_name."</p>";
            echo "<p class=\"p5\">产品描述：".$svc_desc."</p>";
            echo "<p class=\"p5\">服务价格：".$svc_price."元"."</p>";
        ?>
        </div>
        <div style="float:left;width:40%;height:200px;">            
            <img style="margin-top:50px;margin-left:20px;" src="<?php echo base_url();?>css/images/alipay3.jpg" width="128">
            <p style="line-height:20px;font-size:12px;color:green;">支付宝可同时支持支付宝账户、各大网银等多种付款方式！</p>
        </div>
        <br>
        <div class="svc-order-control">
            <a href="<?php echo base_url();?>service"><img src="<?php echo base_url();?>css/images/btn_back.jpg"></a>
            <a href="<?php echo base_url();?>service/checkout"><img src="<?php echo base_url();?>css/images/btn_next.jpg"></a>
        </div>
    </div>
</div>

<?php $this->load->view('footer');?>