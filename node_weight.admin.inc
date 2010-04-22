<?php
/**
 * Settings form
 */
function node_weight_admin_settings_form(){
	$form['node_weight_allow_type'] = array(
		'#type' => 'checkboxes',
		'#title' => t('Allow content type'),
		'#options' => node_type_get_names(),
		'#default_value' => variable_get('node_weight_allow_type', array()),
		'#description' => t('Select content type to use node weight manager.'),
	);
	$form['submit'] = array(
	  '#type' => 'submit',
	  '#value' => t('Save'),
	);
	return $form;
}

/**
 * Settings form submit
 */
function node_weight_admin_settings_form_submit($form, $form_state){
	variable_set('node_weight_allow_type', $form_state['values']['node_weight_allow_type']);
	foreach ($form_state['values']['node_weight_allow_type'] as $type => $value) {
		if ($value) {
			db_query("INSERT INTO {node_weight} SELECT n.nid, 0, CONCAT('node/', n.nid) FROM {node} AS n WHERE n.type = ':type' ON DUPLICATE KEY UPDATE node_weight.nid = node_weight.nid", array(':type' => $type));
		}
	}
}

/**
 * Build the Node weight select type overview form.
 * Loads all node and builds an overview form with weight elements
 *
 * @ingroup forms
 * @see theme_node_weight_type_form()
 */
function node_weight_type_form() {
	$type = variable_get('node_weight_allow_type', NULL);
	$rows = array();
	if ($type) {
		foreach ($type as $key => $value) {
			if ($value) {
				$rows[] = array(array('data' => l($key, 'admin/content/node_weight/'.$key)));
			}
		}
	}
	if (count($rows) > 0) {
		$header = array(t('Content Type'));
		return theme('table', array(
		  'header' => $header, 
		  'rows' => $rows, 
		  'attributes' => array('id' => 'node-weight-type')
		));
	}
	else {
		return 'No content type to select, you can setting '. l('here', 'admin/settings/node_weight', array('query' => drupal_get_destination()));
	}
}

/**
 * Build the Node weight overview form.
 * Loads all node and builds an overview form with weight elements
 *
 * @ingroup forms
 * @see _node_weight_overview_field()
 * @see node_weight_overview_form_submit()
 * @see theme_node_weight_overview_form()
 */
function node_weight_overview_form(&$form_state, $type = NULL) {
  $node_type = arg(3);
	drupal_set_title('Weight Manager for "'.$node_type.'"');
	$allow_type = variable_get('node_weight_allow_type', NULL);
	if ($allow_type) {
		foreach ($allow_type as $allow => $value) {
			if ($value) {
				$allow_types[] = $allow;
			}
		}
		if (isset($allow_types) && in_array($node_type, $allow_types)) {
		  $nodes = array();
			$result = db_query("SELECT n.nid, n.title, n.type, nw.weight FROM {node} AS n LEFT JOIN {node_weight} AS nw ON n.nid = nw.nid WHERE n.type = '$node_type' ORDER BY nw.weight");
			foreach ($result as $node) {
				$nodes[$node->nid] = $node;
			}
			$form['node_weight']['#tree'] = TRUE;
			$form['create_new'] = array(
			  '#type' => 'markup',
			  '#markup' => l('Create new '.$node_type, 'node/add/'.str_replace('_', '-', $node_type), array('query' => drupal_get_destination()))
			);
			if ($nodes) {
				foreach ($nodes as $nid => $node) {
					$form['node_weight'][$nid] = _node_weight_overview_field($node);
				}
				$form['buttons']['submit'] = array(
					'#type' => 'submit',
					'#value' => t('Save changes'),
					'#disabled' => empty($nodes),
				);
			}
			if (!isset($form)) {
				drupal_goto('node_weight');
			}
			return $form;
		}
		else {
		  drupal_goto('node_weight');
		}
  }
  else {
		$form['error'] = array(
			'#type' => 'markup', 
			'#markup' => 'No content type to select, you can setting '. l('here', 'admin/settings/node_weight', array('query' => drupal_get_destination())),
		);
		return $form;
	}
}

/**
 * Build the overview form fields.
 *
 * This internal function should not be called outside the node_weight module,
 *
 * @ingroup forms
 * @see node_weight_overview_form()
 */
function _node_weight_overview_field($node) {
	$form['nid'] = array(
		'#type' => 'hidden',
		'#value' => $node->nid,
	);
	$form['title'] = array(
		'#type' => 'markup',
		'#markup' => l($node->title, 'node/'.$node->nid),
	);
	/*
	$form['taxonomy'] = array(
		'#type' => 'markup',
		'#value' => implode(', ', array_keys((array)taxonomy_node_get_terms(node_load($node->nid), 'name'))),
	);
	*/
	$form['weight'] = array(
		'#type' => 'weight',
		'#default_value' => $node->weight,
	);
	$form['edit'] = array(
		'#type' => 'markup',
		'#markup' => l('edit', 'node/'.$node->nid.'/edit', array('query' => drupal_get_destination())),
	);
	$form['delete'] = array(
		'#type' => 'markup',
		'#markup' => l('delete', 'node/'.$node->nid.'/delete', array('query' => drupal_get_destination())),
	);
	return $form;
}


/**
 * General submit handler for node_weight's overview form.
 *
 * Updates the weights of all nodes on the form.
 *
 * @ingroup formapi
 * @see node_weight_overview_form()
 */
function node_weight_overview_form_submit($form, &$form_state) {
	$nodes = $form_state['values']['node_weight'];
	foreach($nodes as $node) {
	  db_update("node_weight")
	    ->fields(array(
	      'weight' => $node['weight']
	    ))
	    ->condition('nid',$node['nid'])
	    ->execute();
	}
}
