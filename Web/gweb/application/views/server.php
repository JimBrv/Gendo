<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
    $this->load->view('header');
?>

<div class="server-center">
    <div class="div-sub-header" style="margin-top:20px;margin-left:30px;">
	     <div class="div-title20">
	         <strong style="color:#878787"> 服务器列表 </strong>
	     </div>
    </div>
    
	<div style="font-size:14px;margin-left:5%;">
		<p>所有服务器均按照网页浏览、游戏、视频进行类型划分，再按照所处区域进行划分，用户可选择符合自己特定需求的服务器。</p>
		<p>我们特意将大部分IP地址屏蔽了，众所周知，IP地址暴露越多，亲爱的X君就越疯狂。移动终端用户请自行输入IP地址。</p>
		<p>服务器列表会不定期更新，使用客户端软件会自动获取最新的节点信息。</p>
		<p>L2TP协议密码统一为: <img src="<?php echo base_url();?>css/images/l2tppwd.jpg" style="width:80px;" /></p>  
	</div>
	
	<div class="server-list" style="width:90;margin-left:5%;">
	    <table class="svc-tb">
	    	<tr class="svc-tr0">
	    	    <td width="8"><strong></strong></td>  
                <td width="200"><strong>线路名称</strong></td>
	            <td width="280"><strong>描述</strong></td>
	            <td width="50"><strong>区域</strong></td>  
	            <td width="120"><strong>类型</strong></td>
	            <td width="120"><strong>IP地址</strong></td>
	            <td width="200"><strong>支持协议</strong></td>
	            <td width="50"><strong>状态</strong></td>
	    	</tr> 
	    	<?php
	    	    $ip_show = 0;
	    	    foreach($svr_list as $key =>$row) {
	    	    	echo "<tr class=\"svc-tr1\">";
	    	    	if ($row['type'] == 0) {
	    	    		echo "<td width=\"8\" style=\"background:red;\">"."</td>";
	    	    	}else{
	    	    		echo "<td width=\"8\" style=\"background:#4BB2F6;\">"."</td>";
	    	    	}
	    	    	echo "<td width=\"200\">".$row['name']."</td>";
	    	    	echo "<td width=\"280\">".$row['info']."</td>";
	    	    	echo "<td width=\"50\">".$row['location']."</td>";
	    	    	$func = "";
	    	    	if ($row['type'] & 1) {
	    	    	    $func .= "网页/视频(内->外)";
	    	    	}	    	    	
	    	    	if ($row['type'] & 2) {
	    	    	    $func .= "游戏(内->外)";
	    	    	}
	    	    	if ($row['type'] & 4) {	    	    	    
	    	    	    $func .= "网页/视频(外->内)";
	    	    	}
	    	    	if ($row['type'] & 8) {
    	    	        $func .= "游戏(外->内)";
	    	    	}
	    	    	
                    /* 0 to All */
	    	    	if ($row['type'] == 0) {
	    	    	    $func .= "网页/视频/下载";
	    	    	}
	    	    	
	    	    	echo "<td width=\"120\">".$func."</td>";
	    	    	
	    	    	if ($row['type'] == 0) {
	    	    	    /* 免费服务器，显示IP */
	    	    	    if (isset($this->session->userdata['username'])) {
	    	    	        echo "<td width=\"120\">".$row['ip']."</td>";
	    	    	    }else{
	    	    	        echo "<td width=\"120\">"."登录后可见"."</td>";
	    	    	    }
	    	    	}elseif (isset($this->session->userdata['username']) && 
	    	    	         $this->session->userdata['level'] > 0) 
	    	    	{
	    	    	    /* VIP服务器，IP保护，显示一部分 */
	    	    	    if ($ip_show < 2) {
	    	    	        echo "<td width=\"120\">".$row['ip']."</td>";
	    	    	        $ip_show++;
	    	    	    }else{
	    	    	        echo "<td width=\"120\">"."保护资源"."</td>";
	    	    	        $ip_show++;
	    	    	    }
	    	    	}else{
	    	    	    echo "<td width=\"120\">"."仅对VIP用户开放"."</td>";
	    	    	}
	    	    	$proto = "";
	    	    	if ($row['protocol'] & 1) {
	    	    	    $proto .= "PPTP/";
	    	    	}
	    	    	if ($row['protocol'] & 2) {
	    	    	    $proto .= "L2TP/";
	    	    	}	    	    	
	    	    	if ($row['protocol'] & 4 || $row['protocol'] & 8) {
	    	    	    $proto .= "SSL";
	    	    	}
	    	    	echo "<td width=\"200\">".$proto."</td>";
	    	    	//echo "<td width=\"50\">".$row['cur_user']."/".$row['max_user']."</td>";
	    	    	$ratio = $row['cur_user']/$row['max_user'];
	    	    	if ($ratio >= 0.5) {
	    	    	    echo "<td width=\"50\" style=\"color:red;\"><strong>"."拥塞"."</strong></td>";
	    	    	}elseif (0.2 < $ratio && $ratio < 0.5) {
	    	    	    echo "<td width=\"50\" style=\"color:#4BB2F6;\"><strong>"."正常"."</strong></td>";
	    	    	}elseif ($ratio <= 0.2) {
	    	    	    echo "<td width=\"50\" style=\"color:green;\"><strong>"."空闲"."</strong></td>";
	    	    	}
	    	    	echo "</tr>";
	    	    }
	    	?>  
	    </table>
	</div>
</div>


<?php $this->load->view('footer');?>
