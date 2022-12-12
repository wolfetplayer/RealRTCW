using System.Windows.Forms;

namespace BSPCUI
{
    partial class MainForm
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.btn_run = new System.Windows.Forms.Button();
            this.chkb_breadthfirst = new System.Windows.Forms.CheckBox();
            this.chkb_neverbose = new System.Windows.Forms.CheckBox();
            this.chkb_optimize = new System.Windows.Forms.CheckBox();
            this.chkb_nobrushmerge = new System.Windows.Forms.CheckBox();
            this.chkb_noliquids = new System.Windows.Forms.CheckBox();
            this.chkb_writeaasmap = new System.Windows.Forms.CheckBox();
            this.chkb_grapplereach = new System.Windows.Forms.CheckBox();
            this.chkb_freetree = new System.Windows.Forms.CheckBox();
            this.chkb_nocsg = new System.Windows.Forms.CheckBox();
            this.chkb_forcesidesvisible = new System.Windows.Forms.CheckBox();
            this.btn_bsp2aas = new System.Windows.Forms.Button();
            this.btn_cfg1 = new System.Windows.Forms.Button();
            this.btn_output = new System.Windows.Forms.Button();
            this.txtb_bsp2aas = new System.Windows.Forms.TextBox();
            this.txtb_output = new System.Windows.Forms.TextBox();
            this.txtb_cfg1 = new System.Windows.Forms.TextBox();
            this.txtb_cfg2 = new System.Windows.Forms.TextBox();
            this.btn_cfg2 = new System.Windows.Forms.Button();
            this.counter_threads = new System.Windows.Forms.NumericUpDown();
            this.lbl_threads = new System.Windows.Forms.Label();
            this.chkb_cfg1 = new System.Windows.Forms.CheckBox();
            this.chkb_cfg2 = new System.Windows.Forms.CheckBox();
            ((System.ComponentModel.ISupportInitialize)(this.counter_threads)).BeginInit();
            this.SuspendLayout();
            // 
            // btn_run
            // 
            this.btn_run.BackColor = System.Drawing.SystemColors.Control;
            this.btn_run.ForeColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.btn_run.Location = new System.Drawing.Point(404, 288);
            this.btn_run.Name = "btn_run";
            this.btn_run.Size = new System.Drawing.Size(118, 37);
            this.btn_run.TabIndex = 0;
            this.btn_run.Text = "Run";
            this.btn_run.UseVisualStyleBackColor = false;
            this.btn_run.Click += new System.EventHandler(this.run_Click);
            // 
            // chkb_breadthfirst
            // 
            this.chkb_breadthfirst.Font = new System.Drawing.Font("Segoe UI", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.chkb_breadthfirst.Location = new System.Drawing.Point(12, 235);
            this.chkb_breadthfirst.Name = "chkb_breadthfirst";
            this.chkb_breadthfirst.Size = new System.Drawing.Size(130, 26);
            this.chkb_breadthfirst.TabIndex = 1;
            this.chkb_breadthfirst.Text = "breadthfirst";
            this.chkb_breadthfirst.UseVisualStyleBackColor = true;
            // 
            // chkb_neverbose
            // 
            this.chkb_neverbose.Font = new System.Drawing.Font("Segoe UI", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.chkb_neverbose.Location = new System.Drawing.Point(12, 203);
            this.chkb_neverbose.Name = "chkb_neverbose";
            this.chkb_neverbose.Size = new System.Drawing.Size(130, 26);
            this.chkb_neverbose.TabIndex = 2;
            this.chkb_neverbose.Text = "neverbose";
            this.chkb_neverbose.UseVisualStyleBackColor = true;
            // 
            // chkb_optimize
            // 
            this.chkb_optimize.Font = new System.Drawing.Font("Segoe UI", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.chkb_optimize.Location = new System.Drawing.Point(12, 171);
            this.chkb_optimize.Name = "chkb_optimize";
            this.chkb_optimize.Size = new System.Drawing.Size(130, 26);
            this.chkb_optimize.TabIndex = 3;
            this.chkb_optimize.Text = "optimize";
            this.chkb_optimize.UseVisualStyleBackColor = true;
            // 
            // chkb_nobrushmerge
            // 
            this.chkb_nobrushmerge.Font = new System.Drawing.Font("Segoe UI", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.chkb_nobrushmerge.Location = new System.Drawing.Point(12, 267);
            this.chkb_nobrushmerge.Name = "chkb_nobrushmerge";
            this.chkb_nobrushmerge.Size = new System.Drawing.Size(130, 26);
            this.chkb_nobrushmerge.TabIndex = 4;
            this.chkb_nobrushmerge.Text = "nobrushmerge";
            this.chkb_nobrushmerge.UseVisualStyleBackColor = true;
            // 
            // chkb_noliquids
            // 
            this.chkb_noliquids.Font = new System.Drawing.Font("Segoe UI", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.chkb_noliquids.Location = new System.Drawing.Point(12, 299);
            this.chkb_noliquids.Name = "chkb_noliquids";
            this.chkb_noliquids.Size = new System.Drawing.Size(130, 26);
            this.chkb_noliquids.TabIndex = 5;
            this.chkb_noliquids.Text = "noliquids";
            this.chkb_noliquids.UseVisualStyleBackColor = true;
            // 
            // chkb_writeaasmap
            // 
            this.chkb_writeaasmap.Font = new System.Drawing.Font("Segoe UI", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.chkb_writeaasmap.Location = new System.Drawing.Point(148, 299);
            this.chkb_writeaasmap.Name = "chkb_writeaasmap";
            this.chkb_writeaasmap.Size = new System.Drawing.Size(140, 26);
            this.chkb_writeaasmap.TabIndex = 10;
            this.chkb_writeaasmap.Text = "writeaasmap";
            this.chkb_writeaasmap.UseVisualStyleBackColor = true;
            // 
            // chkb_grapplereach
            // 
            this.chkb_grapplereach.Font = new System.Drawing.Font("Segoe UI", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.chkb_grapplereach.Location = new System.Drawing.Point(148, 267);
            this.chkb_grapplereach.Name = "chkb_grapplereach";
            this.chkb_grapplereach.Size = new System.Drawing.Size(140, 26);
            this.chkb_grapplereach.TabIndex = 9;
            this.chkb_grapplereach.Text = "grapplereach";
            this.chkb_grapplereach.UseVisualStyleBackColor = true;
            // 
            // chkb_freetree
            // 
            this.chkb_freetree.Font = new System.Drawing.Font("Segoe UI", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.chkb_freetree.Location = new System.Drawing.Point(148, 171);
            this.chkb_freetree.Name = "chkb_freetree";
            this.chkb_freetree.Size = new System.Drawing.Size(140, 26);
            this.chkb_freetree.TabIndex = 8;
            this.chkb_freetree.Text = "freetree";
            this.chkb_freetree.UseVisualStyleBackColor = true;
            // 
            // chkb_nocsg
            // 
            this.chkb_nocsg.Font = new System.Drawing.Font("Segoe UI", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.chkb_nocsg.Location = new System.Drawing.Point(148, 203);
            this.chkb_nocsg.Name = "chkb_nocsg";
            this.chkb_nocsg.Size = new System.Drawing.Size(140, 26);
            this.chkb_nocsg.TabIndex = 7;
            this.chkb_nocsg.Text = "nocsg";
            this.chkb_nocsg.UseVisualStyleBackColor = true;
            // 
            // chkb_forcesidesvisible
            // 
            this.chkb_forcesidesvisible.Font = new System.Drawing.Font("Segoe UI", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.chkb_forcesidesvisible.Location = new System.Drawing.Point(148, 235);
            this.chkb_forcesidesvisible.Name = "chkb_forcesidesvisible";
            this.chkb_forcesidesvisible.Size = new System.Drawing.Size(140, 26);
            this.chkb_forcesidesvisible.TabIndex = 6;
            this.chkb_forcesidesvisible.Text = "forcesidesvisible";
            this.chkb_forcesidesvisible.UseVisualStyleBackColor = true;
            // 
            // btn_bsp2aas
            // 
            this.btn_bsp2aas.Location = new System.Drawing.Point(473, 12);
            this.btn_bsp2aas.Name = "btn_bsp2aas";
            this.btn_bsp2aas.Size = new System.Drawing.Size(60, 23);
            this.btn_bsp2aas.TabIndex = 11;
            this.btn_bsp2aas.Text = "bsp2aas";
            this.btn_bsp2aas.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.btn_bsp2aas.UseVisualStyleBackColor = true;
            this.btn_bsp2aas.Click += new System.EventHandler(this.bsp2aas_Click);
            // 
            // btn_cfg1
            // 
            this.btn_cfg1.Location = new System.Drawing.Point(473, 69);
            this.btn_cfg1.Name = "btn_cfg1";
            this.btn_cfg1.Size = new System.Drawing.Size(40, 23);
            this.btn_cfg1.TabIndex = 13;
            this.btn_cfg1.Text = "cfg1";
            this.btn_cfg1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.btn_cfg1.UseVisualStyleBackColor = true;
            this.btn_cfg1.Click += new System.EventHandler(this.cfg1_Click);
            // 
            // btn_output
            // 
            this.btn_output.Location = new System.Drawing.Point(473, 41);
            this.btn_output.Name = "btn_output";
            this.btn_output.Size = new System.Drawing.Size(60, 23);
            this.btn_output.TabIndex = 16;
            this.btn_output.Text = "output";
            this.btn_output.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.btn_output.UseVisualStyleBackColor = true;
            this.btn_output.Click += new System.EventHandler(this.output_Click);
            // 
            // txtb_bsp2aas
            // 
            this.txtb_bsp2aas.Location = new System.Drawing.Point(12, 12);
            this.txtb_bsp2aas.Name = "txtb_bsp2aas";
            this.txtb_bsp2aas.Size = new System.Drawing.Size(455, 23);
            this.txtb_bsp2aas.TabIndex = 14;
            // 
            // txtb_output
            // 
            this.txtb_output.Location = new System.Drawing.Point(12, 41);
            this.txtb_output.Name = "txtb_output";
            this.txtb_output.Size = new System.Drawing.Size(455, 23);
            this.txtb_output.TabIndex = 17;
            // 
            // txtb_cfg1
            // 
            this.txtb_cfg1.Location = new System.Drawing.Point(12, 70);
            this.txtb_cfg1.Name = "txtb_cfg1";
            this.txtb_cfg1.Size = new System.Drawing.Size(455, 23);
            this.txtb_cfg1.TabIndex = 18;
            // 
            // txtb_cfg2
            // 
            this.txtb_cfg2.Enabled = false;
            this.txtb_cfg2.Location = new System.Drawing.Point(12, 99);
            this.txtb_cfg2.Name = "txtb_cfg2";
            this.txtb_cfg2.Size = new System.Drawing.Size(455, 23);
            this.txtb_cfg2.TabIndex = 20;
            // 
            // btn_cfg2
            // 
            this.btn_cfg2.Enabled = false;
            this.btn_cfg2.Location = new System.Drawing.Point(473, 98);
            this.btn_cfg2.Name = "btn_cfg2";
            this.btn_cfg2.Size = new System.Drawing.Size(40, 23);
            this.btn_cfg2.TabIndex = 19;
            this.btn_cfg2.Text = "cfg2";
            this.btn_cfg2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.btn_cfg2.UseVisualStyleBackColor = true;
            this.btn_cfg2.Click += new System.EventHandler(this.cfg2_Click);
            // 
            // counter_threads
            // 
            this.counter_threads.Location = new System.Drawing.Point(96, 132);
            this.counter_threads.Maximum = new decimal(new int[] {
            32,
            0,
            0,
            0});
            this.counter_threads.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.counter_threads.Name = "counter_threads";
            this.counter_threads.Size = new System.Drawing.Size(46, 23);
            this.counter_threads.TabIndex = 21;
            this.counter_threads.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // lbl_threads
            // 
            this.lbl_threads.Location = new System.Drawing.Point(12, 130);
            this.lbl_threads.Name = "lbl_threads";
            this.lbl_threads.Size = new System.Drawing.Size(78, 22);
            this.lbl_threads.TabIndex = 23;
            this.lbl_threads.Text = "Threads";
            this.lbl_threads.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // chkb_cfg1
            // 
            this.chkb_cfg1.AutoSize = true;
            this.chkb_cfg1.Checked = true;
            this.chkb_cfg1.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chkb_cfg1.Location = new System.Drawing.Point(518, 74);
            this.chkb_cfg1.Name = "chkb_cfg1";
            this.chkb_cfg1.Size = new System.Drawing.Size(15, 14);
            this.chkb_cfg1.TabIndex = 24;
            this.chkb_cfg1.UseVisualStyleBackColor = true;
            this.chkb_cfg1.CheckedChanged += new System.EventHandler(this.chkb_cfg1_CheckedChanged);
            // 
            // chkb_cfg2
            // 
            this.chkb_cfg2.AutoSize = true;
            this.chkb_cfg2.Location = new System.Drawing.Point(518, 103);
            this.chkb_cfg2.Name = "chkb_cfg2";
            this.chkb_cfg2.Size = new System.Drawing.Size(15, 14);
            this.chkb_cfg2.TabIndex = 25;
            this.chkb_cfg2.UseVisualStyleBackColor = true;
            this.chkb_cfg2.CheckedChanged += new System.EventHandler(this.chkb_cfg2_CheckedChanged);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.ButtonHighlight;
            this.ClientSize = new System.Drawing.Size(547, 342);
            this.Controls.Add(this.chkb_cfg2);
            this.Controls.Add(this.chkb_cfg1);
            this.Controls.Add(this.lbl_threads);
            this.Controls.Add(this.counter_threads);
            this.Controls.Add(this.txtb_cfg2);
            this.Controls.Add(this.btn_cfg1);
            this.Controls.Add(this.btn_cfg2);
            this.Controls.Add(this.txtb_cfg1);
            this.Controls.Add(this.txtb_output);
            this.Controls.Add(this.btn_output);
            this.Controls.Add(this.txtb_bsp2aas);
            this.Controls.Add(this.btn_bsp2aas);
            this.Controls.Add(this.chkb_writeaasmap);
            this.Controls.Add(this.chkb_grapplereach);
            this.Controls.Add(this.chkb_freetree);
            this.Controls.Add(this.chkb_nocsg);
            this.Controls.Add(this.chkb_forcesidesvisible);
            this.Controls.Add(this.chkb_noliquids);
            this.Controls.Add(this.chkb_nobrushmerge);
            this.Controls.Add(this.chkb_optimize);
            this.Controls.Add(this.chkb_neverbose);
            this.Controls.Add(this.chkb_breadthfirst);
            this.Controls.Add(this.btn_run);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "MainForm";
            this.Text = "BSPCUI";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            ((System.ComponentModel.ISupportInitialize)(this.counter_threads)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private CheckBox chkb_breadthfirst;
        private CheckBox chkb_neverbose;
        private CheckBox chkb_optimize;
        private CheckBox chkb_nobrushmerge;
        private CheckBox chkb_noliquids;
        private CheckBox chkb_writeaasmap;
        private CheckBox chkb_grapplereach;
        private CheckBox chkb_freetree;
        private CheckBox chkb_nocsg;
        private CheckBox chkb_forcesidesvisible;

        private Button btn_bsp2aas;
        private Button btn_output;
        private Button btn_cfg1;
        private Button btn_cfg2;
        private Button btn_run;

        private TextBox txtb_bsp2aas;
        private TextBox txtb_output;
        private TextBox txtb_cfg1;
        private TextBox txtb_cfg2;

        private NumericUpDown counter_threads;

        private Label lbl_threads;
        private CheckBox chkb_cfg1;
        private CheckBox chkb_cfg2;
    }
}

