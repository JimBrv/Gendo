<?php if (!defined('BASEPATH')) exit('No direct script access allowed');

class User_mdl extends CI_Model
{
	/* Table db */
	const TBL_RADIUS_USER       = 'userinfo';
	const TBL_RADIUS_USER_CHECK = 'radcheck';
	const TBL_RADIUS_USER_GROUP = 'radusergroup';
	const TBL_OVPN_USER         = 'user';
	/* Radius group */
	const TBL_RADIUS_GROUP_FREE = 'user';
	const TBL_RADIUS_GROUP_VIP  = 'vip';
	
    private $db_radius;
    private $db_ovpn;
    
	/* default we load gendo openvpn database, not radius DB */
	public function __construct()
	{
		$this->db_radius = $this->load->database('radius', TRUE);
		$this->db_ovpn   = $this->load->database('openvpn', TRUE);
	}
	
    public function add_radius_user($name, $pwd, $month_traffic)
	{
		// add to userinfo table	
		$user = array('username'       => "$name",
		              'firstname'      => '',
		              'lastname'       => '',
		              'email'          => '',
		              'department'     => '',
		              'company'        => '',
		              'workphone'      => '',
				      'homephone'      => '',
		              'mobilephone'    => '',
		              'address'        => '',
		              'city'           => '',
		              'state'          => '',
		              'zip'            => '',
					  'notes'          => '',
		              'changeuserinfo' => '0',
		              'creationdate'   => date("Y-m-d H:i:s"),
		              'creationby'     => 'gendo',
		              'updatedate'     => date("Y-m-d H:i:s"),
		              'updateby'       => '',
		               );
		
		$this->db_radius->insert(self::TBL_RADIUS_USER,  $user);		
		if ($this->db_radius->affected_rows() !=1) {
		    log_message('error', "add user=$name fail DB");
		    return -1;	
		}
		
		// add to radusercheck table
		$user_check = array('username' => $name,
		                    'attribute'=> 'Cleartext-Password',
		                    'op'       => ':=',
		                    'value'    => "$pwd");
		$this->db_radius->insert(self::TBL_RADIUS_USER_CHECK,  $user_check);			
		if ($this->db_radius->affected_rows() !=1) {
		    log_message('error', "add user=$user to radcheck fail DB");
		    return -1;	
		}
		
		// add to radusergroup table
	    $user_group = array('username' => "$name", 
	                        'groupname'=> self::TBL_RADIUS_GROUP_FREE,
	                        'priority' => '0');
		$this->db_radius->insert(self::TBL_RADIUS_USER_GROUP,  $user_group);			
		if ($this->db_radius->affected_rows() !=1) {
		    log_message('error', "add user=$user to radusergroup fail DB");
		    return -1;
		}
		
		log_message('info', "add user=$name RADIUS ok!");
		return 0;
	}
	
	public function del_radius_user($name) 
	{
		if (!$this->is_user_exist($name)) {
		    log_message('error', "del user=$name not exist in DB");
		    return -1;
		}
	    $this->db_radius->delete(self::TBL_RADIUS_USER, array('username' => $name)); 
	    $this->db_radius->delete(self::TBL_RADIUS_USER_CHECK, array('username' => $name));
	    $this->db_radius->delete(self::TBL_RADIUS_USER_GROUP, array('username' => $name)); 
	    log_message('info', "del user=$name RADIUS ok!");
		return ($this->db_radius->affected_rows() > 0) ? 0 : -1;
	}	

	
	
	public function add_openvpn_user($name, $pwd, $email, $nick, $refid, $quota_name, $month_traffic)
	{
		$user = array('username'    => "$name",
		              'password'    => "$pwd",
		              'active'      => 1,
		              'creation'    => date("Y-m-d H:i:s"),
		              'name'        => "$nick",
		              'email'       => "$email",
		              'note'        => '',
		              'quota_cycle' => 30,
		              'quota_bytes' => $month_traffic,
				      'quota_name'  => "$quota_name",
				      'quota_start' => date("Y-m-d H:i:s"),
		              'quota_expire'=> date("Y-m-d H:i:s", mktime(0,0,0,date('m'),date('d'),date('Y')+3)),
		              'enabled'     => 1,
		              'level'       => 0,
		              'login_latest'=> date("Y-m-d H:i:s"),
		              'refered'     => $refid,
		               );
		
		$this->db_ovpn->insert(self::TBL_OVPN_USER,  $user);		
		if ($this->db_ovpn->affected_rows() !=1) {
		    log_message('error', 'add user=$user fail on ovpn_db');
		    return -1;	
		}
	    log_message('info', "add user=$name OVPN OK");
		return 0;
	}
	
	public function del_openvpn_user($name) 
	{
		if (!$this->is_user_exist($name)) {
		    log_message('error', "del user=$name not exist in DB");
		    return -1;
		}
	    $this->db_ovpn->delete(self::TBL_OVPN_USER, array('username' => $name)); 
	    log_message('info', "del user=$name OVPN OK");
		return ($this->db_ovpn->affected_rows() > 0) ? 0 : -1;
	}	
	
	public function add_user($name, $pwd, $email, $nick, 
	                         $refid = 0,
	                         $quota_name = '免费用户，流量5G/月，不限时!', 
	                         $month_traffic = 5368709120,
	                         $type = 'free')
	{
	    log_message('error', "Add user=$name($pwd)...");
		if ($this->is_user_exist($name)) {
			log_message('error', "add user=$name existed!");
			return array('ret' => -1, 'error' => '用户已经存在');
		}
	    if ($this->add_openvpn_user($name, $pwd, $email, $nick, $refid, $quota_name, $month_traffic)) {
	    	return array('ret' => -1, 'error' => '加入OpenVPN失败');
	    }
	    
	    if ($this->add_radius_user($name, $pwd, $month_traffic)) {
	    	$this->del_openvpn_user($name);	    	
	    	return array('ret' => -1, 'error' => '加入RADIUS失败');
	    }
	       
		return array('ret' => 0, 'error' => '创建成功');
	}
	
	public function is_user_exist($name)
	{
	    $this->db_ovpn->select('id');
		$this->db_ovpn->where('username', $name);
		
		$query = $this->db_ovpn->get(self::TBL_OVPN_USER);
		$num   = $query->num_rows();
		$query->free_result();
		return $num == 0 ? FALSE : TRUE;
	}
	
    public function is_email_exist($email)
	{
	    $this->db_ovpn->select('id');
		$this->db_ovpn->where('email', $email);
		
		$query = $this->db_ovpn->get(self::TBL_OVPN_USER);
		$num   = $query->num_rows();
		$query->free_result();
		return $num == 0 ? FALSE : TRUE;
	}
	
	public function validate_user($username, $password)
	{
		$data = FALSE;

		$this->db_ovpn->where('username', $username);		
		$this->db_ovpn->where('password', $password);
		$query = $this->db_ovpn->get(self::TBL_OVPN_USER);		
		$num   = $query->num_rows();
		if($num == 1) {
            $data = $query->row_array();
        }
		$query->free_result();
		
		return $data;	    
	}
	
	public function mod_user($name, $pwd, $email, $nick)
	{
	 	log_message('error', "Mod user=$name:$pwd:$email");
		if (!$this->is_user_exist($name)) {
			log_message('error', "Mod user=$name not existed!");
			return array('ret' => -1, 'error' => '用户不存在');
		}
		$data = array('password' => $pwd,
		              'email'    => $email,
		              'name'     => $nick);
		              
		$this->db_ovpn->where('username', $name);
		$query = $this->db_ovpn->update(self::TBL_OVPN_USER, $data);		
	    return array('ret' => 0, 'error' => '修改成功');
	}
	
	public function mod_user_nick($name, $nick)
	{
	 	log_message('error', "Mod nick user=$name");
		if (!$this->is_user_exist($name)) {
			log_message('error', "Mod user=$name not existed!");
			return array('ret' => -1, 'error' => '用户不存在');
		}
		$data = array('name'     => $nick,);
		              
		$this->db_ovpn->where('username', $name);
		$query = $this->db_ovpn->update(self::TBL_OVPN_USER, $data);		
	    return array('ret' => 0, 'error' => '修改成功');
	}
	
	public function mod_user_email($name, $email)
	{
	 	log_message('error', "Mod email=$name:$email");
		if (!$this->is_user_exist($name)) {
			log_message('error', "Mod user=$name not existed!");
			return array('ret' => -1, 'error' => '用户不存在');
		}
		$data = array('email' => $email,);
		              
		$this->db_ovpn->where('username', $name);
		$query = $this->db_ovpn->update(self::TBL_OVPN_USER, $data);		
	    return array('ret' => 0, 'error' => '修改成功');
	}
	
	public function mod_user_password($name, $password)
	{
	 	log_message('error', "Mod pass=$name:$password");
		if (!$this->is_user_exist($name)) {
			log_message('error', "Mod user=$name not existed!");
			return array('ret' => -1, 'error' => '用户不存在');
		}
		$data = array('password' => $password,);

		/* change ovpn table */ 
		$this->db_ovpn->where('username', $name);
		$query = $this->db_ovpn->update(self::TBL_OVPN_USER, $data);
		
		/* change radius table */
		$data = array('value' => $password,);
		$this->db_radius->where('username', $name);
	    $query = $this->db_radius->update(self::TBL_RADIUS_USER_CHECK, $data);
	    
	    return array('ret' => 0, 'error' => '修改成功');
	}
	
	/*
	 * called by user registration OK
	 */
	function update_ref($userid, $bonus)
	{
	 	log_message('error', "AffUser = [$userid, $bonus]");

	 	if ($bonus != 0) {
	 	    $sql = "update user set bonus_cnt=bonus_cnt+1, bonus=bonus+$bonus where id=$userid";
	 	}else{
	 	    $sql = "update user set bonus_cnt=bonus_cnt+1 where id=$userid";
	 	}
		              
		$this->db_ovpn->query($sql);
		if ($this->db_ovpn->affected_rows() != 1) {
		    log_message('error', "AffUser = [$userid, $bonus] failed!!!!");
		}else{
		    log_message('error', "AffUser = [$userid, $bonus] OK!");
		}
	}

	/* 
	 * userid = client's id 
	 * refer's id store in refid feild
	 * called after payment platform callback 
	 */
	function update_bonus($refee_id, $bonus)
	{
	 	log_message('error', "Update bonus, user=[$refee_id], bonus=[$bonus]");
	 	
	 	$this->db_ovpn->where('id', $refee_id);
	 	$query = $this->db_ovpn->get(self::TBL_OVPN_USER);
	 	$user  = $query->row_array();
	 	$query->free_result();
	 	
	 	$refer_id = $user['refered'];        
	 	if ($refer_id <= 0) {
	 	    log_message('error', "Update bonus fail, user=[$refee_id], no refer id found!");
	 	    return;
	 	}
	 	
	 	if ($bonus < 0 || $bonus > 45) {
	 	    log_message('error', "Update bonus fail, user=[$refee_id], bonus=[$bonus] invalid, check system!!!");
	 	    return;	 	    
	 	}
	 	
	 	$sql = "update user set bonus=bonus+$bonus where id=$refer_id";
		              
		$this->db_ovpn->query($sql);
		if ($this->db_ovpn->affected_rows() != 1) {
		    log_message('error', "Update bonus fail, user=[$refee_id], ref-user=[$refer_id], bonus=[$bonus]");
		}else{
		    log_message('error', "Update bonus OK, user=[$refee_id], ref-user=[$refer_id], bonus=[$bonus]");
		}
	}

	
	
	
	/*
	* quota_cycle, quota_bytes should be disprecated for VIP 
	*  and quota_expire takes effect to determine user's service time 
	* change user to VIP after fund gendo :)
	*
	*/	
	function update_quota($order_no,
	                      $uid,
	                      $quota_name, 
	                      $quota_cycle = 30,
	                      $quota_bytes = 10737418240, 
	                      $quota_start, 
	                      $quota_expire, 
	                      $quota_price)
	{
	    $sql = "update user set quota_name='$quota_name', quota_bytes=$quota_bytes, quota_used=0,".
	           "quota_start='$quota_start', quota_expire='$quota_expire', fund=fund+$quota_price,".
	           "enabled=1, level=1, active=1 ".
	           "where id=$uid";
		              
		$this->db_ovpn->query($sql);
		if ($this->db_ovpn->affected_rows() != 1) {
		    log_message('error', "Notify update user quota, order=[$order_no], user_id=[$uid] failed!");
		}else{
		    log_message('error', "Notify update user quota, order=[$order_no], user_id=[$uid] OK");
		}	    
	}
	
	function update_login($uid)
	{
	    $sql = "update user set login_cnt=login_cnt+1, login_latest=CURRENT_TIMESTAMP ".
	           "where id=$uid";
		              
		$this->db_ovpn->query($sql);
		if ($this->db_ovpn->affected_rows() != 1) {
		    log_message('error', "Notify update user last_login, user_id=[$uid] failed!");
		}	    
	}
}
